
#include "ALAGE/utils/Logger.h"

namespace alag
{

template<typename AssetType>
AssetHandler<AssetType>::AssetHandler()
{
    m_loadThread = new sf::Thread(&AssetHandler<AssetType>::LoadInThread, this);
    m_curNewId = 0;
}


template<typename AssetType>
AssetHandler<AssetType>::~AssetHandler()
{
    CleanAll();
    m_loadThread->terminate();
    delete m_loadThread;
}



template<typename AssetType>
AssetType* AssetHandler<AssetType>::GetAsset(const AssetTypeID& assetID)
{
    typename std::map<AssetTypeID, AssetType*>::iterator assetIter;

    assetIter = m_assets.find(assetID);

    if(assetIter == m_assets.end())
    {
        Logger::Error("Cannot find asset");
        return (nullptr);
    }

    return assetIter->second;
}


template<typename AssetType>
AssetType* AssetHandler<AssetType>::LoadAssetFromFile(
                                        const std::string &filePath, AssetLoadType loadType)
{
    return LoadAssetFromFile(GenerateID(), filePath, loadType);
}

template<typename AssetType>
AssetType* AssetHandler<AssetType>::LoadAssetFromFile(const AssetTypeID& id,
                                        const std::string &filePath, AssetLoadType loadType)
{
	std::map<std::string, AssetTypeID>::iterator filesIter;
	filesIter = m_filesList.find(filePath);
	if(filesIter != m_filesList.end())
		return GetAsset(filesIter->second);

    AssetType* newAsset = AddAsset(id);
    newAsset->LoadFromFile(filePath, loadType);
    if(loadType == LoadTypeNow)
        newAsset->LoadNow();
    else
        AddToLoadingThread(newAsset);

	m_filesList[filePath] = id;

    return newAsset;
}


template<typename AssetType>
AssetType* AssetHandler<AssetType>::LoadAssetFromMemory(
                                        void *data, std::size_t dataSize, AssetLoadType loadType)
{
    return LoadAssetFromMemory(GenerateID(), data, dataSize, loadType);
}

template<typename AssetType>
AssetType* AssetHandler<AssetType>::LoadAssetFromMemory(const AssetTypeID& id,
                                        void *data, std::size_t dataSize, AssetLoadType loadType)
{
    Asset* newAsset = AddAsset(id);
    newAsset->LoadFromMemory(data, dataSize, loadType);
    if(loadType == LoadTypeNow)
        newAsset->LoadNow();
    else
        AddToLoadingThread(newAsset);

    return newAsset;
}


template<typename AssetType>
AssetType* AssetHandler<AssetType>::LoadAssetFromStream(
                                        sf::InputStream *stream, AssetLoadType loadType)
{
    return LoadAssetFromStream(GenerateID(), stream, loadType);
}

template<typename AssetType>
AssetType* AssetHandler<AssetType>::LoadAssetFromStream(const AssetTypeID& id,
                                        sf::InputStream *stream, AssetLoadType loadType)
{
    Asset* newAsset = AddAsset(id);
    newAsset->LoadFromStream(stream, loadType);
    if(loadType == LoadTypeNow)
        newAsset->LoadNow();
    else
        AddToLoadingThread(newAsset);

    return newAsset;
}


template<typename AssetType>
AssetType* AssetHandler<AssetType>::AddAsset(const AssetTypeID &assetID, bool plannedObsolescence, int lifeSpan)
{
    sf::Lock lockLoadMutex(m_loadMutex);

    AssetType* newAsset = nullptr;
    typename std::map<AssetTypeID, AssetType*>::iterator assetIter;

    assetIter = m_assets.find(assetID);

    if(assetIter == m_assets.end())
    {
        newAsset = new AssetType(assetID);
        m_assets[assetID] = newAsset;
    } else {
        if(!plannedObsolescence)
            RemoveFromObsolescenceList(assetID);
        newAsset = assetIter->second;
    }

    if(plannedObsolescence)
        AddToObsolescenceList(assetID, lifeSpan);

    return newAsset;
}

template<class AssetType>
AssetTypeID AssetHandler<AssetType>::GenerateID()
{
    return m_curNewId++;
}



template<typename AssetType>
void AssetHandler<AssetType>::AddToLoadingThread(AssetType* asset)
{
    sf::Lock lockLoadMutex(m_loadMutex);

    if(asset != nullptr && !asset->IsLoaded())
    {
        m_assetsToLoadInThread.push_back(asset);
        if(m_assetsToLoadInThread.size() == 1)
            m_loadThread->launch();
    }
}

template<typename AssetType>
void AssetHandler<AssetType>::RemoveFromLoadingThread(AssetType* asset)
{
    sf::Lock lockLoadMutex(m_loadMutex);

    if(asset != nullptr)
    {
        typename std::list<AssetType*>::iterator loadingListIt;
        loadingListIt = m_assetsToLoadInThread.begin();
        while(loadingListIt != m_assetsToLoadInThread.end())
        {
            if(*loadingListIt == asset)
            {
                m_assetsToLoadInThread.erase(loadingListIt);
                loadingListIt = m_assetsToLoadInThread.end();
            } else
                ++loadingListIt;
        }
    }
}

template<typename AssetType>
void AssetHandler<AssetType>:: AddToObsolescenceList(const AssetTypeID &assetID,int lifeSpan)
{
    if(GetAsset(assetID) != nullptr)
        m_obsolescenceList[assetID] = lifeSpan;
}

template<typename AssetType>
void AssetHandler<AssetType>:: RemoveFromObsolescenceList(const AssetTypeID &assetID)
{
    std::map<AssetTypeID, int>::iterator iter;
    iter = m_obsolescenceList.find(assetID);
    if(iter != m_obsolescenceList.end())
        m_obsolescenceList.erase(iter);
}

template<typename AssetType>
void AssetHandler<AssetType>:: DescreaseObsolescenceLife()
{
    std::map<AssetTypeID, int>::iterator iter;
    for(iter = m_obsolescenceList.begin() ; iter != m_obsolescenceList.end() ; ++iter)
    {
        iter->second--;
        if(iter->second-- <= 0)
        {
            DeleteAsset(iter->first);
            m_obsolescenceList.erase(iter);
        }
    }

}

template<typename AssetType>
void AssetHandler<AssetType>::DeleteAsset(const AssetTypeID &assetID)
{
    sf::Lock lockLoadMutex(m_loadMutex);

    typename std::map<AssetTypeID, AssetType*>::iterator iter;
    iter = m_assets.find(assetID);

    if(iter != m_assets.end())
    {
        if(iter->second != nullptr)
        {
            if(!iter->second->GetFilePath().empty())
                m_filesList.erase(m_filesList.find(iter->second->GetFilePath()));

            if(!iter->second->IsLoaded() && iter->second->GetLoadType() == LoadTypeInThread)
                RemoveFromLoadingThread(iter->second);

            delete iter->second;
            m_assets.erase(iter);
        }
    } else
        Logger::Error("Trying to delete unfindable asset");
}


template<typename AssetType>
void AssetHandler<AssetType>::DeleteAsset(AssetType *asset)
{
    if(asset != nullptr)
        DeleteAsset(asset->GetID());
}


template<typename AssetType>
void AssetHandler<AssetType>::CleanAll()
{
    m_loadThread->wait();
    typename std::map<AssetTypeID, AssetType*>::iterator assetIter;
    for(assetIter = m_assets.begin() ; assetIter != m_assets.end() ; ++assetIter)
        delete assetIter->second;
    m_assets.clear();
    m_obsolescenceList.clear();
    m_filesList.clear();
}


template<typename AssetType>
void AssetHandler<AssetType>::LoadInThread()
{
    while(!m_assetsToLoadInThread.empty())
    {
        LockLoadMutex();
        if(m_assetsToLoadInThread.front() != nullptr)
        {
            if(!m_assetsToLoadInThread.front()->IsLoaded())
                m_assetsToLoadInThread.front()->LoadNow();
        } else
            Logger::Error("Tried to load null asset in thread");

        m_assetsToLoadInThread.erase(m_assetsToLoadInThread.begin());
        UnlockLoadMutex();
    }
}


template<typename AssetType>
void AssetHandler<AssetType>::LockLoadMutex()
{
    m_loadMutex.lock();
}

template<typename AssetType>
void AssetHandler<AssetType>::UnlockLoadMutex()
{
    m_loadMutex.unlock();
}

}