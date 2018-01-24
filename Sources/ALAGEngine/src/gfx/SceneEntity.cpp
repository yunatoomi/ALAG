#include "ALAGE/gfx/SceneEntity.h"
#include "ALAGE/gfx/SceneNode.h"
#include "ALAGE/utils/Logger.h"

namespace alag
{

SceneEntity::SceneEntity()
{
    m_isAnEntity = true;
    m_canBeLighted = false;
    m_isLighted = true;
    m_is3D = false;
}

SceneEntity::~SceneEntity()
{
    //dtor
}

bool SceneEntity::CanBeIlluminated()
{
    return m_isLighted & m_canBeLighted;
}

bool SceneEntity::Is3D()
{
    return m_is3D;
}

void SceneEntity::ActivateLighting()
{
    m_isLighted = true;
    if(!m_canBeLighted)
        Logger::Error("Cannot activate illumination");
}

void SceneEntity::DesactivateLighting()
{
    m_isLighted = false;
}

void SceneEntity::PrepareShader(sf::Shader *)
{

}


}
