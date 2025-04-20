def parse_instruction_file(file):
    """
    Parse an instruction file and return a list of instructions
    """
    instructions = []
    try:
        # Read all lines from the file
        content = file.read().decode('utf-8').splitlines()
        
        # Process each line
        for line in content:
            # Remove comments and trim whitespace
            line = line.split('#')[0].strip()
            
            # Skip empty lines
            if not line:
                continue
            
            # Add instruction to list
            instructions.append(line)
        
        return instructions
    except Exception as e:
        print(f"Error parsing instruction file: {e}")
        return []

def parse_data_file(file):
    """
    Parse a data file and return a dictionary of data values
    Format: address: value
    """
    data = {}
    try:
        # Read all lines from the file
        content = file.read().decode('utf-8').splitlines()
        
        # Process each line
        for line in content:
            # Remove comments and trim whitespace
            line = line.split('#')[0].strip()
            
            # Skip empty lines
            if not line:
                continue
            
            # Parse address and value
            parts = line.split(':')
            if len(parts) == 2:
                try:
                    # Parse address
                    addr_str = parts[0].strip()
                    if addr_str.startswith('0x'):
                        address = int(addr_str, 16)
                    else:
                        address = int(addr_str)
                    
                    # Parse value
                    value_str = parts[1].strip()
                    if value_str.startswith('0x'):
                        value = int(value_str, 16)
                    else:
                        value = int(value_str)
                    
                    # Add to data dictionary
                    data[address] = value
                except ValueError:
                    print(f"Error parsing line: {line}")
        
        return data
    except Exception as e:
        print(f"Error parsing data file: {e}")
        return {}
