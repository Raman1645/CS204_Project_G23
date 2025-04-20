from instructions import get_instruction_type, InstructionType, parse_instruction

class PipelineRegister:
    """Represents a pipeline register between two stages"""
    def __init__(self, name):
        self.name = name
        self.pc = None
        self.instruction = None
        self.instruction_number = None
        self.rs1 = None
        self.rs2 = None
        self.rd = None
        self.immediate = None
        self.op_type = None
        self.alu_result = None
        self.memory_data = None
        self.branch_taken = None
        self.branch_target = None
        self.valid = False
    
    def clear(self):
        """Clear the pipeline register"""
        self.pc = None
        self.instruction = None
        self.instruction_number = None
        self.rs1 = None
        self.rs2 = None
        self.rd = None
        self.immediate = None
        self.op_type = None
        self.alu_result = None
        self.memory_data = None
        self.branch_taken = None
        self.branch_target = None
        self.valid = False
    
    def to_dict(self):
        """Convert register contents to dictionary for display"""
        return {
            "name": self.name,
            "pc": self.pc,
            "instruction": self.instruction,
            "valid": self.valid,
            "rs1": self.rs1,
            "rs2": self.rs2,
            "rd": self.rd,
            "immediate": self.immediate,
            "op_type": self.op_type,
            "alu_result": self.alu_result,
            "memory_data": self.memory_data,
            "branch_taken": self.branch_taken,
            "branch_target": self.branch_target
        }

class Pipeline:
    def __init__(self, text_memory, data_memory, enable_pipeline=True, enable_forwarding=True):
        # Memories
        self.text_memory = text_memory
        self.data_memory = data_memory
        
        # Configuration
        self.enable_pipeline = enable_pipeline
        self.enable_forwarding = enable_forwarding
        
        # Register file (32 general-purpose registers)
        self.registers = [0] * 32
        
        # Program counter
        self.pc = 0
        
        # Pipeline registers
        self.if_id = PipelineRegister("IF/ID")
        self.id_ex = PipelineRegister("ID/EX")
        self.ex_mem = PipelineRegister("EX/MEM")
        self.mem_wb = PipelineRegister("MEM/WB")
        
        # Hazard detection unit
        self.stall = False
        self.flush = False
        self.forward_ex_ex_rs1 = False
        self.forward_ex_ex_rs2 = False
        self.forward_mem_ex_rs1 = False
        self.forward_mem_ex_rs2 = False
        
        # Statistics
        self.instruction_count = 0
        
        # Current pipeline state for visualization
        self.current_pipeline_state = {
            "Fetch": None,
            "Decode": None,
            "Execute": None,
            "Memory": None,
            "Writeback": None
        }
        
        # Active forwarding paths for visualization
        self.forwarding_paths = []
    
    def execute_cycle(self, branch_predictor):
        """Execute one pipeline cycle"""
        # Initialize result dictionary
        result = {
            "finished": False,
            "instruction_completed": False,
            "completed_instruction_type": None,
            "current_instruction": None,
            "current_pc": None,
            "current_stage": None,
            "instruction_number": None,
            "data_hazard": False,
            "control_hazard": False,
            "hazard_description": None,
            "stall": self.stall,
            "stall_reason": None,
            "flush": self.flush,
            "branch_misprediction": False
        }
        
        # Clear forwarding paths
        self.forwarding_paths = []
        
        # If pipelining is disabled, execute in the single-cycle mode
        if not self.enable_pipeline:
            return self._execute_single_cycle(result)
        
        # Perform pipeline stages in reverse order to avoid data overwriting
        # Writeback Stage
        wb_result = self._writeback_stage()
        if wb_result:
            result.update(wb_result)
        
        # Memory Stage
        mem_result = self._memory_stage()
        if mem_result:
            result.update(mem_result)
        
        # Execute Stage
        ex_result = self._execute_stage(branch_predictor)
        if ex_result:
            result.update(ex_result)
        
        # Update hazard detection
        hazard_result = self._detect_hazards()
        if hazard_result:
            result.update(hazard_result)
        
        # Decode Stage
        id_result = self._decode_stage()
        if id_result:
            result.update(id_result)
        
        # Fetch Stage
        if_result = self._fetch_stage(branch_predictor)
        if if_result:
            result.update(if_result)
        
        # Check if simulation is finished
        if self.pc >= len(self.text_memory.memory) and not self.if_id.valid and not self.id_ex.valid and not self.ex_mem.valid and not self.mem_wb.valid:
            result["finished"] = True
        
        return result
    
    def _execute_single_cycle(self, result):
        """Execute in single-cycle mode (non-pipelined)"""
        # Fetch instruction
        if self.pc >= len(self.text_memory.memory):
            result["finished"] = True
            return result
        
        instruction = self.text_memory.read(self.pc)
        result["current_instruction"] = instruction
        result["current_pc"] = self.pc
        result["instruction_number"] = self.instruction_count + 1
        
        # Parse instruction
        parsed = parse_instruction(instruction)
        if not parsed:
            self.pc += 4
            return result
        
        op_type = parsed["op_type"]
        rs1 = parsed.get("rs1")
        rs2 = parsed.get("rs2")
        rd = parsed.get("rd")
        immediate = parsed.get("immediate")
        
        # Read register values
        rs1_val = self.registers[rs1] if rs1 is not None else None
        rs2_val = self.registers[rs2] if rs2 is not None else None
        
        # Execute operation
        alu_result = None
        memory_data = None
        branch_taken = False
        next_pc = self.pc + 4
        
        if op_type == "add":
            alu_result = rs1_val + rs2_val
        elif op_type == "sub":
            alu_result = rs1_val - rs2_val
        elif op_type == "and":
            alu_result = rs1_val & rs2_val
        elif op_type == "or":
            alu_result = rs1_val | rs2_val
        elif op_type == "xor":
            alu_result = rs1_val ^ rs2_val
        elif op_type == "sll":
            alu_result = rs1_val << rs2_val
        elif op_type == "srl":
            alu_result = rs1_val >> rs2_val
        elif op_type == "lw":
            alu_result = rs1_val + immediate
            memory_data = self.data_memory.read(alu_result)
        elif op_type == "sw":
            alu_result = rs1_val + immediate
            self.data_memory.write(alu_result, rs2_val)
        elif op_type == "beq":
            branch_taken = (rs1_val == rs2_val)
            if branch_taken:
                next_pc = self.pc + immediate
        elif op_type == "bne":
            branch_taken = (rs1_val != rs2_val)
            if branch_taken:
                next_pc = self.pc + immediate
        elif op_type == "j":
            branch_taken = True
            next_pc = self.pc + immediate
        elif op_type == "jal":
            branch_taken = True
            alu_result = self.pc + 4  # Return address
            next_pc = self.pc + immediate
        elif op_type == "jr":
            branch_taken = True
            next_pc = rs1_val
        elif op_type == "jalr":
            branch_taken = True
            alu_result = self.pc + 4  # Return address
            next_pc = rs1_val
        elif op_type == "addi":
            alu_result = rs1_val + immediate
        elif op_type == "andi":
            alu_result = rs1_val & immediate
        elif op_type == "ori":
            alu_result = rs1_val | immediate
        elif op_type == "xori":
            alu_result = rs1_val ^ immediate
        
        # Write result to register if needed
        if rd is not None and op_type not in ["sw", "beq", "bne", "j"]:
            if op_type == "lw":
                self.registers[rd] = memory_data
            else:
                self.registers[rd] = alu_result
        
        # Update program counter
        self.pc = next_pc
        
        # Update statistics
        self.instruction_count += 1
        result["instruction_completed"] = True
        
        instr_type = get_instruction_type(op_type)
        result["completed_instruction_type"] = instr_type
        
        return result
    
    def _fetch_stage(self, branch_predictor):
        """Pipeline Fetch Stage"""
        result = {}
        
        # If stalled, don't fetch new instruction
        if self.stall:
            self.current_pipeline_state["Fetch"] = "Stalled"
            return result
        
        # If end of program, invalidate the stage
        if self.pc >= len(self.text_memory.memory):
            self.current_pipeline_state["Fetch"] = None
            return result
        
        # Fetch instruction from text memory
        instruction = self.text_memory.read(self.pc)
        
        # If pipeline is being flushed, don't update IF/ID register
        if self.flush:
            self.current_pipeline_state["Fetch"] = f"Flushed: {instruction}"
            return result
        
        # Predict branch if it's a branch instruction
        parsed = parse_instruction(instruction)
        if parsed and parsed["op_type"] in ["beq", "bne", "j", "jal", "jr", "jalr"]:
            # Get branch prediction
            prediction = branch_predictor.predict(self.pc)
            next_pc = prediction["predicted_target"] if prediction["prediction"] else self.pc + 4
            
            # Update branch predictor entry
            branch_predictor.update_entry(self.pc, prediction["predicted_target"], prediction["prediction"])
            
            result["branch_prediction"] = {
                "pc": self.pc,
                "prediction": prediction["prediction"],
                "predicted_target": prediction["predicted_target"]
            }
        else:
            next_pc = self.pc + 4
        
        # Update IF/ID pipeline register
        self.if_id.pc = self.pc
        self.if_id.instruction = instruction
        self.if_id.instruction_number = self.instruction_count + 1
        self.if_id.valid = True
        
        # Update PC
        old_pc = self.pc
        self.pc = next_pc
        
        # Update visualization state
        self.current_pipeline_state["Fetch"] = instruction
        
        # Update result
        result["current_instruction"] = instruction
        result["current_pc"] = old_pc
        result["current_stage"] = "Fetch"
        result["instruction_number"] = self.if_id.instruction_number
        
        return result
    
    def _decode_stage(self):
        """Pipeline Decode Stage"""
        result = {}
        
        # If IF/ID register is invalid, nothing to decode
        if not self.if_id.valid:
            self.current_pipeline_state["Decode"] = None
            return result
        
        # Get instruction from IF/ID register
        instruction = self.if_id.instruction
        pc = self.if_id.pc
        
        # Parse instruction
        parsed = parse_instruction(instruction)
        if not parsed:
            # Invalid instruction, propagate without doing anything
            if not self.stall:
                self.id_ex.pc = pc
                self.id_ex.instruction = instruction
                self.id_ex.instruction_number = self.if_id.instruction_number
                self.id_ex.valid = True
                self.if_id.valid = False
            
            self.current_pipeline_state["Decode"] = f"Invalid: {instruction}"
            return result
        
        # Extract instruction fields
        op_type = parsed["op_type"]
        rs1 = parsed.get("rs1")
        rs2 = parsed.get("rs2")
        rd = parsed.get("rd")
        immediate = parsed.get("immediate")
        
        # Read register values
        rs1_val = self.registers[rs1] if rs1 is not None else None
        rs2_val = self.registers[rs2] if rs2 is not None else None
        
        # If stalled, don't update ID/EX register
        if self.stall:
            self.current_pipeline_state["Decode"] = f"Stalled: {instruction}"
            
            result["current_instruction"] = instruction
            result["current_pc"] = pc
            result["current_stage"] = "Decode"
            result["instruction_number"] = self.if_id.instruction_number
            return result
        
        # Update ID/EX pipeline register
        self.id_ex.pc = pc
        self.id_ex.instruction = instruction
        self.id_ex.instruction_number = self.if_id.instruction_number
        self.id_ex.rs1 = rs1
        self.id_ex.rs2 = rs2
        self.id_ex.rd = rd
        self.id_ex.immediate = immediate
        self.id_ex.op_type = op_type
        self.id_ex.rs1_val = rs1_val
        self.id_ex.rs2_val = rs2_val
        self.id_ex.valid = True
        
        # Clear IF/ID register if not stalled
        self.if_id.valid = False
        
        # Update visualization state
        self.current_pipeline_state["Decode"] = instruction
        
        # Update result
        result["current_instruction"] = instruction
        result["current_pc"] = pc
        result["current_stage"] = "Decode"
        result["instruction_number"] = self.id_ex.instruction_number
        
        return result
    
    def _execute_stage(self, branch_predictor):
        """Pipeline Execute Stage"""
        result = {}
        
        # If ID/EX register is invalid, nothing to execute
        if not self.id_ex.valid:
            self.current_pipeline_state["Execute"] = None
            return result
        
        # Get values from ID/EX register
        pc = self.id_ex.pc
        instruction = self.id_ex.instruction
        op_type = self.id_ex.op_type
        rs1 = self.id_ex.rs1
        rs2 = self.id_ex.rs2
        rd = self.id_ex.rd
        immediate = self.id_ex.immediate
        rs1_val = self.id_ex.rs1_val
        rs2_val = self.id_ex.rs2_val
        
        # Apply forwarding if enabled
        if self.enable_forwarding:
            # Forward from EX/MEM
            if self.forward_ex_ex_rs1:
                rs1_val = self.ex_mem.alu_result
                self.forwarding_paths.append({
                    "from_x": 3, 
                    "from_y": 0.7, 
                    "to_x": 2, 
                    "to_y": 0.3,
                    "description": f"EX/MEM -> EX (RS1: {rs1})"
                })
            
            if self.forward_ex_ex_rs2:
                rs2_val = self.ex_mem.alu_result
                self.forwarding_paths.append({
                    "from_x": 3, 
                    "from_y": 0.7, 
                    "to_x": 2, 
                    "to_y": 0.4,
                    "description": f"EX/MEM -> EX (RS2: {rs2})"
                })
            
            # Forward from MEM/WB
            if self.forward_mem_ex_rs1:
                value_to_forward = self.mem_wb.memory_data if self.mem_wb.op_type == "lw" else self.mem_wb.alu_result
                rs1_val = value_to_forward
                self.forwarding_paths.append({
                    "from_x": 4, 
                    "from_y": 0.7, 
                    "to_x": 2, 
                    "to_y": 0.3,
                    "description": f"MEM/WB -> EX (RS1: {rs1})"
                })
            
            if self.forward_mem_ex_rs2:
                value_to_forward = self.mem_wb.memory_data if self.mem_wb.op_type == "lw" else self.mem_wb.alu_result
                rs2_val = value_to_forward
                self.forwarding_paths.append({
                    "from_x": 4, 
                    "from_y": 0.7, 
                    "to_x": 2, 
                    "to_y": 0.4,
                    "description": f"MEM/WB -> EX (RS2: {rs2})"
                })
        
        # Execute operation
        alu_result = None
        branch_taken = False
        branch_target = None
        
        if op_type == "add":
            alu_result = rs1_val + rs2_val
        elif op_type == "sub":
            alu_result = rs1_val - rs2_val
        elif op_type == "and":
            alu_result = rs1_val & rs2_val
        elif op_type == "or":
            alu_result = rs1_val | rs2_val
        elif op_type == "xor":
            alu_result = rs1_val ^ rs2_val
        elif op_type == "sll":
            alu_result = rs1_val << rs2_val
        elif op_type == "srl":
            alu_result = rs1_val >> rs2_val
        elif op_type == "lw" or op_type == "sw":
            alu_result = rs1_val + immediate
        elif op_type == "beq":
            branch_taken = (rs1_val == rs2_val)
            branch_target = pc + immediate
        elif op_type == "bne":
            branch_taken = (rs1_val != rs2_val)
            branch_target = pc + immediate
        elif op_type == "j":
            branch_taken = True
            branch_target = pc + immediate
        elif op_type == "jal":
            branch_taken = True
            alu_result = pc + 4  # Return address
            branch_target = pc + immediate
        elif op_type == "jr":
            branch_taken = True
            branch_target = rs1_val
        elif op_type == "jalr":
            branch_taken = True
            alu_result = pc + 4  # Return address
            branch_target = rs1_val
        elif op_type == "addi":
            alu_result = rs1_val + immediate
        elif op_type == "andi":
            alu_result = rs1_val & immediate
        elif op_type == "ori":
            alu_result = rs1_val | immediate
        elif op_type == "xori":
            alu_result = rs1_val ^ immediate
        
        # Check branch prediction accuracy
        if op_type in ["beq", "bne", "j", "jal", "jr", "jalr"]:
            # Get recorded prediction
            prediction = branch_predictor.get_prediction(pc)
            was_predicted_taken = prediction["prediction"]
            predicted_target = prediction["predicted_target"]
            
            # Check if prediction was correct
            prediction_correct = (was_predicted_taken == branch_taken) and (not branch_taken or predicted_target == branch_target)
            
            # Update branch predictor with actual outcome
            branch_predictor.update(pc, branch_target, branch_taken)
            
            # If mispredicted, flush the pipeline and update PC
            if not prediction_correct:
                self.flush = True
                self.pc = branch_target if branch_taken else pc + 4
                result["branch_misprediction"] = True
                result["flush"] = True
                result["control_hazard"] = True
                result["hazard_description"] = f"Branch misprediction at PC {pc:04x}"
        
        # Update EX/MEM pipeline register
        self.ex_mem.pc = pc
        self.ex_mem.instruction = instruction
        self.ex_mem.instruction_number = self.id_ex.instruction_number
        self.ex_mem.rd = rd
        self.ex_mem.op_type = op_type
        self.ex_mem.alu_result = alu_result
        self.ex_mem.rs2_val = rs2_val  # For store instructions
        self.ex_mem.branch_taken = branch_taken
        self.ex_mem.branch_target = branch_target
        self.ex_mem.valid = True
        
        # Clear ID/EX register
        self.id_ex.valid = False
        
        # Update visualization state
        self.current_pipeline_state["Execute"] = instruction
        
        # Update result
        result["current_instruction"] = instruction
        result["current_pc"] = pc
        result["current_stage"] = "Execute"
        result["instruction_number"] = self.ex_mem.instruction_number
        
        return result
    
    def _memory_stage(self):
        """Pipeline Memory Stage"""
        result = {}
        
        # If EX/MEM register is invalid, nothing to do
        if not self.ex_mem.valid:
            self.current_pipeline_state["Memory"] = None
            return result
        
        # Get values from EX/MEM register
        pc = self.ex_mem.pc
        instruction = self.ex_mem.instruction
        op_type = self.ex_mem.op_type
        rd = self.ex_mem.rd
        alu_result = self.ex_mem.alu_result
        rs2_val = self.ex_mem.rs2_val
        
        # Memory operation
        memory_data = None
        
        if op_type == "lw":
            memory_data = self.data_memory.read(alu_result)
        elif op_type == "sw":
            self.data_memory.write(alu_result, rs2_val)
        
        # Update MEM/WB pipeline register
        self.mem_wb.pc = pc
        self.mem_wb.instruction = instruction
        self.mem_wb.instruction_number = self.ex_mem.instruction_number
        self.mem_wb.rd = rd
        self.mem_wb.op_type = op_type
        self.mem_wb.alu_result = alu_result
        self.mem_wb.memory_data = memory_data
        self.mem_wb.valid = True
        
        # Clear EX/MEM register
        self.ex_mem.valid = False
        
        # Update visualization state
        self.current_pipeline_state["Memory"] = instruction
        
        # Update result
        result["current_instruction"] = instruction
        result["current_pc"] = pc
        result["current_stage"] = "Memory"
        result["instruction_number"] = self.mem_wb.instruction_number
        
        return result
    
    def _writeback_stage(self):
        """Pipeline Writeback Stage"""
        result = {}
        
        # If MEM/WB register is invalid, nothing to do
        if not self.mem_wb.valid:
            self.current_pipeline_state["Writeback"] = None
            return result
        
        # Get values from MEM/WB register
        pc = self.mem_wb.pc
        instruction = self.mem_wb.instruction
        op_type = self.mem_wb.op_type
        rd = self.mem_wb.rd
        alu_result = self.mem_wb.alu_result
        memory_data = self.mem_wb.memory_data
        
        # Write result to register if needed
        if rd is not None and op_type not in ["sw", "beq", "bne", "j"]:
            if op_type == "lw":
                self.registers[rd] = memory_data
            else:
                self.registers[rd] = alu_result
        
        # Instruction is completed
        self.instruction_count += 1
        result["instruction_completed"] = True
        result["completed_instruction_type"] = get_instruction_type(op_type)
        
        # Clear MEM/WB register
        self.mem_wb.valid = False
        
        # Update visualization state
        self.current_pipeline_state["Writeback"] = instruction
        
        # Update result
        result["current_instruction"] = instruction
        result["current_pc"] = pc
        result["current_stage"] = "Writeback"
        result["instruction_number"] = self.mem_wb.instruction_number
        
        return result
    
    def _detect_hazards(self):
        """Detect and handle hazards in the pipeline"""
        result = {
            "data_hazard": False,
            "control_hazard": False,
            "hazard_description": None,
            "stall": False,
            "stall_reason": None
        }
        
        # Reset forwarding and stall flags
        self.forward_ex_ex_rs1 = False
        self.forward_ex_ex_rs2 = False
        self.forward_mem_ex_rs1 = False
        self.forward_mem_ex_rs2 = False
        self.stall = False
        
        # Check for flush condition from previous cycle
        if self.flush:
            self.flush = False
            self.if_id.clear()  # Clear IF/ID on flush
            return result
        
        # If ID/EX is not valid, no hazards to detect
        if not self.id_ex.valid:
            return result
        
        # Extract registers used in ID/EX
        rs1 = self.id_ex.rs1
        rs2 = self.id_ex.rs2
        id_ex_op_type = self.id_ex.op_type
        
        # Data hazards
        stall_needed = False
        
        # Check for RAW hazards with EX/MEM
        if self.ex_mem.valid and self.ex_mem.rd is not None:
            ex_mem_writes_rd = self.ex_mem.op_type not in ["sw", "beq", "bne", "j"]
            
            if ex_mem_writes_rd and self.ex_mem.rd != 0:
                # RS1 hazard
                if rs1 is not None and rs1 == self.ex_mem.rd:
                    result["data_hazard"] = True
                    if self.enable_forwarding:
                        # Load-use hazard cannot be resolved with forwarding
                        if self.ex_mem.op_type == "lw":
                            stall_needed = True
                            result["hazard_description"] = f"Load-use hazard: {self.ex_mem.instruction} -> {self.id_ex.instruction} (RS1)"
                            result["stall_reason"] = "data_hazard"
                        else:
                            self.forward_ex_ex_rs1 = True
                            result["hazard_description"] = f"RAW hazard with forwarding: {self.ex_mem.instruction} -> {self.id_ex.instruction} (RS1)"
                    else:
                        stall_needed = True
                        result["hazard_description"] = f"RAW hazard without forwarding: {self.ex_mem.instruction} -> {self.id_ex.instruction} (RS1)"
                        result["stall_reason"] = "data_hazard"
                
                # RS2 hazard
                if rs2 is not None and rs2 == self.ex_mem.rd:
                    result["data_hazard"] = True
                    if self.enable_forwarding:
                        # Load-use hazard cannot be resolved with forwarding
                        if self.ex_mem.op_type == "lw":
                            stall_needed = True
                            result["hazard_description"] = f"Load-use hazard: {self.ex_mem.instruction} -> {self.id_ex.instruction} (RS2)"
                            result["stall_reason"] = "data_hazard"
                        else:
                            self.forward_ex_ex_rs2 = True
                            result["hazard_description"] = f"RAW hazard with forwarding: {self.ex_mem.instruction} -> {self.id_ex.instruction} (RS2)"
                    else:
                        stall_needed = True
                        result["hazard_description"] = f"RAW hazard without forwarding: {self.ex_mem.instruction} -> {self.id_ex.instruction} (RS2)"
                        result["stall_reason"] = "data_hazard"
        
        # Check for RAW hazards with MEM/WB
        if self.mem_wb.valid and self.mem_wb.rd is not None and not (self.forward_ex_ex_rs1 or self.forward_ex_ex_rs2):
            mem_wb_writes_rd = self.mem_wb.op_type not in ["sw", "beq", "bne", "j"]
            
            if mem_wb_writes_rd and self.mem_wb.rd != 0:
                # RS1 hazard
                if rs1 is not None and rs1 == self.mem_wb.rd and not self.forward_ex_ex_rs1:
                    result["data_hazard"] = True
                    if self.enable_forwarding:
                        self.forward_mem_ex_rs1 = True
                        result["hazard_description"] = f"RAW hazard with forwarding: {self.mem_wb.instruction} -> {self.id_ex.instruction} (RS1)"
                    else:
                        stall_needed = True
                        result["hazard_description"] = f"RAW hazard without forwarding: {self.mem_wb.instruction} -> {self.id_ex.instruction} (RS1)"
                        result["stall_reason"] = "data_hazard"
                
                # RS2 hazard
                if rs2 is not None and rs2 == self.mem_wb.rd and not self.forward_ex_ex_rs2:
                    result["data_hazard"] = True
                    if self.enable_forwarding:
                        self.forward_mem_ex_rs2 = True
                        result["hazard_description"] = f"RAW hazard with forwarding: {self.mem_wb.instruction} -> {self.id_ex.instruction} (RS2)"
                    else:
                        stall_needed = True
                        result["hazard_description"] = f"RAW hazard without forwarding: {self.mem_wb.instruction} -> {self.id_ex.instruction} (RS2)"
                        result["stall_reason"] = "data_hazard"
        
        # Apply stall if needed
        if stall_needed:
            self.stall = True
            result["stall"] = True
        
        return result
    
    def get_pipeline_state(self):
        """Return the current state of the pipeline for visualization"""
        return self.current_pipeline_state
    
    def get_pipeline_registers(self):
        """Return the contents of pipeline registers for display"""
        return [
            self.if_id.to_dict(),
            self.id_ex.to_dict(),
            self.ex_mem.to_dict(),
            self.mem_wb.to_dict()
        ]
    
    def get_register_file(self):
        """Return the register file contents for display"""
        reg_file = []
        for i in range(32):
            reg_file.append({
                "register": f"x{i}",
                "name": f"{'zero' if i == 0 else f'x{i}'}",
                "value": self.registers[i]
            })
        return reg_file
    
    def get_forwarding_paths(self):
        """Return the active forwarding paths for visualization"""
        return self.forwarding_paths
