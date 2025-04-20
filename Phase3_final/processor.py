from pipeline import Pipeline
from memory import Memory
from branch_predictor import BranchPredictor
from instructions import InstructionType, get_instruction_type

class Processor:
    def __init__(self, instructions, data, enable_pipeline=True, enable_forwarding=True, 
                 enable_reg_trace=False, enable_pipeline_trace=False, 
                 trace_instruction=None, enable_branch_trace=False):
        # Configuration
        self.enable_pipeline = enable_pipeline
        self.enable_forwarding = enable_forwarding
        self.enable_reg_trace = enable_reg_trace
        self.enable_pipeline_trace = enable_pipeline_trace
        self.trace_instruction = trace_instruction
        self.enable_branch_trace = enable_branch_trace
        
        # Memory initialization
        self.text_memory = Memory()
        self.data_memory = Memory()
        
        # Load instructions and data into memory
        self.text_memory.load(instructions)
        self.data_memory.load(data)
        
        # Pipeline
        self.pipeline = Pipeline(self.text_memory, self.data_memory, 
                                enable_pipeline=enable_pipeline,
                                enable_forwarding=enable_forwarding)
        
        # Branch predictor
        self.branch_predictor = BranchPredictor()
        
        # Statistics
        self.cycle_count = 0
        self.instruction_count = 0
        self.data_transfer_instructions = 0
        self.alu_instructions = 0
        self.control_instructions = 0
        self.stall_count = 0
        self.data_hazards = 0
        self.control_hazards = 0
        self.branch_mispredictions = 0
        self.stalls_data_hazards = 0
        self.stalls_control_hazards = 0
        
        # Instruction trace
        self.instruction_trace = []
        self.hazards = []
        
        # Flag to indicate if simulation is finished
        self.finished = False
    
    def execute_cycle(self):
        """Execute one processor cycle and update statistics"""
        # If simulation already finished, return True
        if self.finished:
            return True
        
        # Increment cycle count
        self.cycle_count += 1
        
        # Execute one pipeline cycle
        cycle_result = self.pipeline.execute_cycle(self.branch_predictor)
        
        # Update statistics based on cycle result
        if cycle_result.get("finished", False):
            self.finished = True
        
        # Update instruction count if an instruction completed
        if cycle_result.get("instruction_completed", False):
            self.instruction_count += 1
            
            # Update instruction type counts
            instr_type = cycle_result.get("completed_instruction_type")
            if instr_type == InstructionType.DATA_TRANSFER:
                self.data_transfer_instructions += 1
            elif instr_type == InstructionType.ALU:
                self.alu_instructions += 1
            elif instr_type == InstructionType.CONTROL:
                self.control_instructions += 1
        
        # Update hazard statistics
        if cycle_result.get("data_hazard", False):
            self.data_hazards += 1
            
            # Record hazard information
            self.hazards.append({
                "cycle": self.cycle_count,
                "type": "Data Hazard",
                "description": cycle_result.get("hazard_description", ""),
                "stall": cycle_result.get("stall", False)
            })
            
        if cycle_result.get("control_hazard", False):
            self.control_hazards += 1
            
            # Record hazard information
            self.hazards.append({
                "cycle": self.cycle_count,
                "type": "Control Hazard",
                "description": cycle_result.get("hazard_description", ""),
                "stall": cycle_result.get("stall", False)
            })
        
        # Update stall statistics
        if cycle_result.get("stall", False):
            self.stall_count += 1
            
            if cycle_result.get("stall_reason") == "data_hazard":
                self.stalls_data_hazards += 1
            elif cycle_result.get("stall_reason") == "control_hazard":
                self.stalls_control_hazards += 1
        
        # Update branch misprediction count
        if cycle_result.get("branch_misprediction", False):
            self.branch_mispredictions += 1
        
        # Update instruction trace
        if cycle_result.get("current_instruction"):
            # Add to instruction trace
            instr_info = {
                "cycle": self.cycle_count,
                "pc": cycle_result.get("current_pc"),
                "instruction": cycle_result.get("current_instruction"),
                "stage": cycle_result.get("current_stage"),
                "stall": cycle_result.get("stall", False),
                "flush": cycle_result.get("flush", False)
            }
            
            # Only track traced instruction if specified
            if self.trace_instruction is None or self.trace_instruction == cycle_result.get("instruction_number"):
                self.instruction_trace.append(instr_info)
        
        return self.finished
    
    def get_statistics(self):
        """Return the simulation statistics"""
        cpi = self.cycle_count / max(1, self.instruction_count)
        
        return {
            "total_cycles": self.cycle_count,
            "total_instructions": self.instruction_count,
            "cpi": cpi,
            "data_transfer_instructions": self.data_transfer_instructions,
            "alu_instructions": self.alu_instructions,
            "control_instructions": self.control_instructions,
            "total_stalls": self.stall_count,
            "data_hazards": self.data_hazards,
            "control_hazards": self.control_hazards,
            "branch_mispredictions": self.branch_mispredictions,
            "stalls_data_hazards": self.stalls_data_hazards,
            "stalls_control_hazards": self.stalls_control_hazards
        }
    
    def get_pipeline_state(self):
        """Return the current state of the pipeline for visualization"""
        return self.pipeline.get_pipeline_state()
    
    def get_pipeline_registers(self):
        """Return the contents of pipeline registers"""
        if not self.enable_pipeline_trace:
            return None
        
        return self.pipeline.get_pipeline_registers()
    
    def get_register_file(self):
        """Return the register file contents"""
        if not self.enable_reg_trace:
            return None
        
        return self.pipeline.get_register_file()
    
    def get_instruction_trace(self):
        """Return the instruction trace"""
        return self.instruction_trace
    
    def get_hazards(self):
        """Return the hazards encountered"""
        return self.hazards
    
    def get_branch_predictor_state(self):
        """Return the state of the branch predictor"""
        if not self.enable_branch_trace:
            return None
        
        return self.branch_predictor.get_state()
    
    def get_forwarding_paths(self):
        """Return the active forwarding paths for visualization"""
        if not self.enable_forwarding:
            return []
        
        return self.pipeline.get_forwarding_paths()
