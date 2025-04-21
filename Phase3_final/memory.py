class Memory:
    """
    Class representing a memory segment (either text or data)
    """
    def __init__(self):
        self.memory = {}
    
    def read(self, address):
        """Read from memory at the given address"""
        return self.memory.get(address, 0)
    
    def write(self, address, data):
        """Write data to memory at the given address"""
        self.memory[address] = data
    
    def load(self, data):
        """Load multiple values into memory"""
        if isinstance(data, dict):
            # If data is a dictionary, merge it with memory
            self.memory.update(data)
        elif isinstance(data, list):
            # If data is a list, store each element at consecutive addresses
            for i, value in enumerate(data):
                self.memory[i * 4] = value
                
