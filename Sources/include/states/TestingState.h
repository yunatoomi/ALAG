#ifndef TESTINGSTATE_H
#define TESTINGSTATE_H

#include "ALAGE/core/GState.h"
#include "ALAGE/utils/singleton.h"
#include "ALAGE/gfx/IsometricScene.h"


class TestingState : public alag::GState, public Singleton<TestingState>
{
    friend class Singleton<TestingState>;

    public:
        friend TestingState* Singleton<TestingState>::Instance();

        void Entered();
        void Leaving();
        void Revealed();
        void Obscuring();

        void HandleEvents(alag::EventManager*);
        void Update(sf::Time);
        void Draw(sf::RenderTarget*);

        void Init();

    protected:
        TestingState();
        virtual ~TestingState();

    private:
        sf::Time m_totalTime;
        bool m_firstEntering;
        bool showfirstsecond;

        alag::IsometricScene m_mainScene;

        sf::Vector2f m_camMove;

        alag::SceneNode *m_sarcoNode;
};

#endif // TESTINGSTATE_H