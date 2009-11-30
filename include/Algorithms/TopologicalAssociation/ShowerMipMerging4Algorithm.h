/**
 *  @file   PandoraPFANew/include/Algorithms/TopologicalAssociation/ShowerMipMerging4Algorithm.h
 * 
 *  @brief  Header file for the shower mip merging 4 algorithm class.
 * 
 *  $Log: $
 */
#ifndef SHOWER_MIP_MERGING_4_ALGORITHM_H
#define SHOWER_MIP_MERGING_4_ALGORITHM_H 1

#include "Algorithms/Algorithm.h"

/**
 *  @brief  ShowerMipMerging4Algorithm class
 */
class ShowerMipMerging4Algorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        Algorithm *CreateAlgorithm() const;
    };

private:
    StatusCode Run();
    StatusCode ReadSettings(const TiXmlHandle xmlHandle);

    /**
     *  @brief  Get the smallest distance between a hit in a cluster and the projected initial direction of a second cluster.
     * 
     *  @param  pClusterToProject address of the cluster to project
     *  @param  pClusterToExamine address of the cluster to examine
     * 
     *  @return the smallest distance from the initial projection
     */
    float GetDistanceFromInitialProjection(const pandora::Cluster *const pClusterToProject, const pandora::Cluster *const pClusterToExamine) const;

    static const float FLOAT_MAX;

    float           m_canMergeMinMipFraction;           ///< The min mip fraction for clusters (flagged as photons) to be merged
    float           m_canMergeMaxRms;                   ///< The max all hit fit rms for clusters (flagged as photons) to be merged

    unsigned int    m_minCaloHitsPerDaughterCluster;    ///< The min number of calo hits for cluster to be used as a daughter cluster
    unsigned int    m_minCaloHitsPerParentCluster;      ///< The min number of calo hits for cluster to be used as a parent cluster

    unsigned int    m_maxLayerDifference;               ///< The max difference between parent outer and daughter inner pseudo layers

    float           m_maxProjectionDistance;            ///< Max value of distance between daughter cluster hits and parent projection
    float           m_maxProjectionDistanceRatio;       ///< Max ratio (perp./parallel distances) to allow projection distance calculation

    float           m_maxIntraLayerDistance;            ///< Max value of closest intra layer approach between parent and daughter clusters
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *ShowerMipMerging4Algorithm::Factory::CreateAlgorithm() const
{
    return new ShowerMipMerging4Algorithm();
}

#endif // #ifndef SHOWER_MIP_MERGING_4_ALGORITHM_H