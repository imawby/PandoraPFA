/**
 *	@file PandoraPFANew/src/PandoraPFANewProcessor.cc
 * 
 *	@brief Implementation of the pandora pfa new processor class.
 * 
 *	$Log: $
 */

#include "EVENT/CalorimeterHit.h"
#include "EVENT/LCCollection.h"
#include "EVENT/Track.h"

#include "UTIL/CellIDDecoder.h"

// User algorithm includes here


#include "Api/PandoraApi.h"

#include "PandoraPFANewProcessor.h"

PandoraPFANewProcessor pandoraPFANewProcessor;

//------------------------------------------------------------------------------------------------------------------------------------------

PandoraPFANewProcessor::PandoraPFANewProcessor() :
	Processor("PandoraPFANewProcessor"),
	m_nRun(0),
	m_nEvent(0)
{
	_description = "Pandora reconstructs clusters and particle flow objects";
	
	this->ProcessSteeringFile();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PandoraPFANewProcessor::init()
{
	try
	{
		std::cout << "PandoraPFANewProcessor - Init" << std::endl;
		
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, this->CreateGeometry());
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, this->RegisterUserAlgorithmFactories());
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(m_pandora, m_settings.m_pandoraSettingsXmlFile));
	}
	catch (StatusCodeException &statusCodeException)
	{
		std::cout << "Failed to initialize pandora pfa new processor: " << statusCodeException.ToString() << std::endl;
		throw;
	}
	catch (...)
	{
		std::cout << "Failed to initialize pandora pfa new processor, unrecognized exception" << std::endl;
		throw;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PandoraPFANewProcessor::processRunHeader(LCRunHeader *pLCRunHeader)
{
	m_detectorName = pLCRunHeader->getDetectorName();
	std::cout << "Detector Name " << m_detectorName << ", Run " << ++m_nRun <<  std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PandoraPFANewProcessor::processEvent(LCEvent *pLCEvent)
{
	try
	{
		std::cout << "Run " << m_nRun << ", Event " << ++m_nEvent << std::endl;

		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, this->CreateTracks(pLCEvent));
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, this->CreateCaloHits(pLCEvent));
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(m_pandora));
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, this->ProcessParticleFlowObjects(pLCEvent));
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(m_pandora));
	}
	catch (StatusCodeException &statusCodeException)
	{
		std::cout << "Failed to process event: " << statusCodeException.ToString() << std::endl;
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(m_pandora));
	}
	catch (...)
	{
		std::cout << "Failed to process event, unrecognized exception" << std::endl;
		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(m_pandora));		
	}	
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PandoraPFANewProcessor::check(LCEvent *pLCEvent)
{
	std::cout << "PandoraPFANewProcessor - Check" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PandoraPFANewProcessor::end()
{
	std::cout << "PandoraPFANewProcessor - End" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PandoraPFANewProcessor::CreateGeometry()
{
	// Insert user code here ...
	PandoraApi::Geometry::Parameters geometryParameters;
	geometryParameters.m_tpcInnerRadius = 10;
	PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Geometry::Create(m_pandora, geometryParameters));

	return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PandoraPFANewProcessor::RegisterUserAlgorithmFactories()
{
	// Insert user code here ...

	return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
	
StatusCode PandoraPFANewProcessor::CreateTracks(const LCEvent *const pLCEvent)
{
	// Insert user code here ...
	for (StringVector::const_iterator iter = m_settings.m_trackCollections.begin(), 
		iterEnd = m_settings.m_trackCollections.end(); iter != iterEnd; ++iter)
	{
		try
		{
			const LCCollection *pTrackCollection = pLCEvent->getCollection(*iter);
			
			for (int i = 0; i < pTrackCollection->getNumberOfElements(); ++i)
			{
				Track* pTrack = dynamic_cast<Track*>(pTrackCollection->getElementAt(i));
				
				PandoraApi::Track::Parameters trackParameters;
				trackParameters.m_momentum = 10;
				trackParameters.m_pParentAddress = pTrack;

				PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Track::Create(m_pandora, trackParameters));
			}
		}
		catch (StatusCodeException &statusCodeException)
		{
			std::cout << "Failed to extract a track: " << statusCodeException.ToString() << std::endl;
		}
		catch (...)
		{
			std::cout << "Failed to extract a track, unrecognised exception" << std::endl;
		}
	}

	return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PandoraPFANewProcessor::CreateCaloHits(const LCEvent *const pLCEvent)
{
	// Insert user code here ...
	for (StringVector::const_iterator iter = m_settings.m_hCalCollections.begin(), 
		iterEnd = m_settings.m_hCalCollections.end(); iter != iterEnd; ++iter)
	{
		try
		{
			CellIDDecoder<CalorimeterHit>::setDefaultEncoding("M:3,S-1:3,I:9,J:9,K-1:6");

			const LCCollection *pCaloHitCollection = pLCEvent->getCollection(*iter);
			CellIDDecoder<CalorimeterHit> cellIdDecoder(pCaloHitCollection);

			for (int i = 0; i < pCaloHitCollection->getNumberOfElements(); ++i)
			{
				CalorimeterHit* pCaloHit = dynamic_cast<CalorimeterHit*>(pCaloHitCollection->getElementAt(i));

				PandoraApi::CaloHit::Parameters caloHitParameters;
				caloHitParameters.m_energy = pCaloHit->getEnergy();
				caloHitParameters.m_pParentAddress = pCaloHit;
				caloHitParameters.m_layer = cellIdDecoder(pCaloHit)["K-1"];

				PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::CaloHit::Create(m_pandora, caloHitParameters));
			}
		}
		catch (StatusCodeException &statusCodeException)
		{
			std::cout << "Failed to extract a calo hit: " << statusCodeException.ToString() << std::endl;
		}
		catch (...)
		{
			std::cout << "Failed to extract a calo hit, unrecognised exception" << std::endl;
		}
	}

	return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PandoraPFANewProcessor::ProcessParticleFlowObjects(const LCEvent *const pLCEvent)
{
	// Insert user code here ...
	PandoraApi::ParticleFlowObjectList particleFlowObjectList;
	PANDORA_THROW_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=,
		PandoraApi::GetParticleFlowObjects(m_pandora, particleFlowObjectList));

	return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
	
StatusCode PandoraPFANewProcessor::CreateMCTrees(const LCEvent *const pLCEvent)
{
	// Insert user code here ...

	return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PandoraPFANewProcessor::ProcessSteeringFile()
{
	// Insert user code here ...
	registerProcessorParameter("PandoraSettingsXmlFile",
							"The pandora settings xml file",
							m_settings.m_pandoraSettingsXmlFile,
							std::string());

	// Input collections
	registerInputCollections(LCIO::TRACK,
							"TrackCollections", 
							"Names of the Track collections used for clustering",
							m_settings.m_trackCollections,
							StringVector(1, std::string("LDCTracks")));	
	
	registerInputCollections(LCIO::TRACK,
							"TrackCollections", 
							"Names of the Track collections used for clustering",
							m_settings.m_trackCollections,
							StringVector(1, std::string("LDCTracks")));

	registerInputCollections(LCIO::VERTEX,
							"V0VertexCollections", 
							"Name of external V0 Vertex collections",
							m_settings.m_v0VertexCollections,
							StringVector(1, std::string("V0Vertices")));

	registerInputCollections(LCIO::CALORIMETERHIT,
							"CaloHitcollections", 
							"Name of the HCAL collection used to form clusters",
							m_settings.m_hCalCollections,
							StringVector(1, std::string("HCAL")));
}

