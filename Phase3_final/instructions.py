from enum import Enum

class InstructionType(Enum):
    DATA_TRANSFER = "Data Transfer"
    ALU = "ALU"
    CONTROL = "Control"

def get_instruction_type(op_type):
    """Determine the type of an instruction based on its operation"""
    if op_type in ["lw", "sw"]:
        return InstructionType.DATA_TRANSFER
    elif op_type in ["beq", "bne", "j", "jal", "jr", "jalr"]:
        return InstructionType.CONTROL
    else:
        return InstructionType.ALU

def parse_instruction(instruction):
    """Parse an instruction string into its components"""
    if not isinstance(instruction, str):
        return None
        
    
    # Trim whitespace and split by spaces
    parts = instruction.strip().split()
    if not parts:
        return None
    
    # Get operation
    op = parts[0].lower()
    
    # Initialize result
    result = {"op_type": op}
    
    # Parse based on instruction type
    if op in ["add", "sub", "and", "or", "xor", "sll", "srl"]:
        # Format: OP rd, rs1, rs2
        if len(parts) != 4:
            return None
        
        rd = parse_register(parts[1])
        rs1 = parse_register(parts[2])
        rs2 = parse_register(parts[3])
        
        result["rd"] = rd
        result["rs1"] = rs1
        result["rs2"] = rs2
    
    elif op in ["addi", "andi", "ori", "xori"]:
        # Format: OP rd, rs1, imm
        if len(parts) != 4:
            return None
        
        rd = parse_register(parts[1])
        rs1 = parse_register(parts[2])
        imm = parse_immediate(parts[3])
        
        result["rd"] = rd
        result["rs1"] = rs1
        result["immediate"] = imm
    
    elif op == "lw":
        # Format: lw rd, imm(rs1)
        if len(parts) != 3:
            return None
        
        rd = parse_register(parts[1])
        addr = parse_address(parts[2])
        if addr is None:
            return None
        
        rs1 = addr["register"]
        imm = addr["offset"]
        
        result["rd"] = rd
        result["rs1"] = rs1
        result["immediate"] = imm
    
    elif op == "sw":
        # Format: sw rs2, imm(rs1)
        if len(parts) != 3:
            return None
        
        rs2 = parse_register(parts[1])
        addr = parse_address(parts[2])
        if addr is None:
            return None
        
        rs1 = addr["register"]
        imm = addr["offset"]
        
        result["rs2"] = rs2
        result["rs1"] = rs1
        result["immediate"] = imm
    
    elif op in ["beq", "bne"]:
        # Format: OP rs1, rs2, label/imm
        if len(parts) != 4:
            return None
        
        rs1 = parse_register(parts[1])
        rs2 = parse_register(parts[2])
        imm = parse_immediate(parts[3])
        
        result["rs1"] = rs1
        result["rs2"] = rs2
        result["immediate"] = imm
    
    elif op == "j" or op == "jal":
        # Format: j label/imm
        # Format: jal label/imm
        if op == "j" and len(parts) != 2:
            return None
        if op == "jal" and len(parts) != 3:
            return None
        
        if op == "jal":
            rd = parse_register(parts[1])
            imm = parse_immediate(parts[2])
            result["rd"] = rd
        else:
            imm = parse_immediate(parts[1])
        
        result["immediate"] = imm
    
    elif op == "jr":
        # Format: jr rs1
        if len(parts) != 2:
            return None
        
        rs1 = parse_register(parts[1])
        result["rs1"] = rs1
    
    elif op == "jalr":
        # Format: jalr rd, rs1
        if len(parts) != 3:
            return None
        
        rd = parse_register(parts[1])
        rs1 = parse_register(parts[2])
        
        result["rd"] = rd
        result["rs1"] = rs1
    
    else:
        # Unknown instruction
        return None
    
    return result

def parse_register(reg_str):
    """Parse a register string and return its number"""
    # Remove any commas
    reg_str = reg_str.rstrip(',')
    
    # Check if it's in the form x0, x1, etc.
    if reg_str.startswith('x'):
        try:
            reg_num = int(reg_str[1:])
            if 0 <= reg_num <= 31:
                return reg_num
        except ValueError:
            pass
    
    # Check for special register names
    reg_map = {
        "zero": 0,
        "ra": 1,
        "sp": 2,
        "gp": 3,
        "tp": 4,
        "t0": 5, "t1": 6, "t2": 7,
        "s0": 8, "fp": 8, "s1": 9,
        "a0": 10, "a1": 11, "a2": 12, "a3": 13, "a4": 14, "a5": 15, "a6": 16, "a7": 17,
        "s2": 18, "s3": 19, "s4": 20, "s5": 21, "s6": 22, "s7": 23, "s8": 24, "s9": 25, "s10": 26, "s11": 27,
        "t3": 28, "t4": 29, "t5": 30, "t6": 31
    }
    
    if reg_str in reg_map:
        return reg_map[reg_str]
    
    # Couldn't parse register
    return None

def parse_immediate(imm_str):
    """Parse an immediate value"""
    # Remove any commas
    imm_str = imm_str.rstrip(',')
    
    try:
        # Try to parse as decimal
        return int(imm_str)
    except ValueError:
        try:
            # Try to parse as hexadecimal
            if imm_str.startswith('0x') or imm_str.startswith('0X'):
                return int(imm_str, 16)
            # Try to parse as binary
            elif imm_str.startswith('0b') or imm_str.startswith('0B'):
                return int(imm_str, 2)
        except ValueError:
            pass
    
    # Couldn't parse immediate
    return 0

def parse_address(addr_str):
    """Parse an address in the form imm(rs1)"""
    # Extract the offset and register parts
    if '(' not in addr_str or ')' not in addr_str:
        return None
    
    # Split into offset and register
    try:
        offset_str = addr_str[:addr_str.index('(')]
        reg_str = addr_str[addr_str.index('(')+1:addr_str.index(')')]
        
        offset = parse_immediate(offset_str) if offset_str else 0
        reg = parse_register(reg_str)
        
        if reg is None:
            return None
        
        return {"offset": offset, "register": reg}
    except:
        return None
