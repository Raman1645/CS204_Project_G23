#include "structs.h"

// Implementation of BranchPredictor methods declared in structs.h

bool BranchPredictor::predict(const std::string &pc)
{
    // Increment the total predictions counter
    predictions++;

    // Check if this branch has been seen before
    if (pht.find(pc) != pht.end())
    {
        // Return the prediction (taken or not taken)
        return pht[pc];
    }
    // Default prediction for branches we haven't seen before (not taken)
    return false;
}

std::string BranchPredictor::getTarget(const std::string &pc)
{
    // Check if we have a target address for this branch
    if (btb.find(pc) != btb.end())
    {
        // Return the target address
        return btb[pc];
    }
    // Return empty string if no target is found
    return "";
}

void BranchPredictor::update(const std::string &pc, bool taken, const std::string &target)
{
    // Check if the prediction was correct
    if (pht.find(pc) != pht.end() && pht[pc] == taken)
    {
        correct_predictions++;
    }

    // Update the Pattern History Table with the actual branch outcome
    pht[pc] = taken;

    // If the branch was taken, update the Branch Target Buffer
    if (taken)
    {
        btb[pc] = target;
    }
}