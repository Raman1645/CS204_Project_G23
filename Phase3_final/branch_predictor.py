class BranchPredictor:
    """
    A 1-bit dynamic branch predictor with a branch target buffer (BTB)
    """
    def __init__(self):
        # Pattern History Table (PHT) - maps PC to prediction bit
        self.pht = {}
        
        # Branch Target Buffer (BTB) - maps PC to target address
        self.btb = {}
    
    def predict(self, pc):
        """
        Predict whether a branch will be taken and what the target will be
        Returns a dictionary with keys:
            - prediction: Boolean indicating if branch is predicted taken
            - predicted_target: The predicted target address if taken
        """
        # Check if we have seen this branch before
        if pc in self.pht:
            is_taken = self.pht[pc] == 1
            target = self.btb.get(pc, pc + 4)
            return {
                "prediction": is_taken,
                "predicted_target": target
            }
        else:
            # Default prediction: not taken
            return {
                "prediction": False,
                "predicted_target": pc + 4
            }
    
    def update(self, pc, target, taken):
        """
        Update the branch predictor with the actual outcome
        """
        # Update PHT: 1-bit predictor simply sets bit to actual outcome
        self.pht[pc] = 1 if taken else 0
        
        # Update BTB if branch was taken
        if taken:
            self.btb[pc] = target
    
    def update_entry(self, pc, target, prediction):
        """
        Update or add an entry to the branch predictor
        Used for initial setup or explicit updates
        """
        self.pht[pc] = 1 if prediction else 0
        if prediction:
            self.btb[pc] = target
    
    def get_prediction(self, pc):
        """
        Get the current prediction for a PC without updating
        Returns same format as predict()
        """
        return self.predict(pc)
    
    def get_state(self):
        """
        Return the current state of the branch predictor for visualization
        """
        # Format PHT for display
        pht_entries = []
        for pc, prediction in self.pht.items():
            pht_entries.append({
                "PC": f"0x{pc:04x}",
                "Prediction": "Taken" if prediction == 1 else "Not Taken"
            })
        
        # Format BTB for display
        btb_entries = []
        for pc, target in self.btb.items():
            btb_entries.append({
                "PC": f"0x{pc:04x}",
                "Target": f"0x{target:04x}"
            })

        
        return {
            "pattern_history_table": pht_entries,
            "branch_target_buffer": btb_entries
        }
