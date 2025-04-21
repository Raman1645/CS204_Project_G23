#include "structs.h"

// Branch prediction implementation

bool BranchPredictor::predict(const std::string &pc)
{
    // Keep track of total branch predictions requested
    predictions++;

    // Check branch history to see if we've encountered this address before
    if (pht.find(pc) != pht.end())
    {
        // Use historical data to make prediction
        return pht[pc];
    }
    
    // For new branch addresses, default to not taken (conservative approach)
    return false;
}

std::string BranchPredictor::getTarget(const std::string &pc)
{
    // Look up target address in branch target buffer
    if (btb.find(pc) != btb.end())
    {
        // Found a previously recorded target address
        return btb[pc];
    }
    
    // No target information available for this branch
    return "";
}

void BranchPredictor::update(const std::string &pc, bool taken, const std::string &target)
{
    // Evaluate prediction accuracy
    if (pht.find(pc) != pht.end() && pht[pc] == taken)
    {
        // Record successful prediction
        correct_predictions++;
    }

    // Update our prediction model with actual outcome
    pht[pc] = taken;

    // Only store target addresses for taken branches
    if (taken)
    {
        btb[pc] = target;
    }
}
