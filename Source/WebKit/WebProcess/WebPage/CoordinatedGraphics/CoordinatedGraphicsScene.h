/*
    Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2013 Company 100, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#if USE(COORDINATED_GRAPHICS)
#include <WebCore/Damage.h>
#include <WebCore/NicosiaPlatformLayer.h>
#include <WebCore/NicosiaScene.h>
#include <WebCore/TextureMapper.h>
#include <WebCore/TextureMapperFPSCounter.h>
#include <WebCore/TextureMapperLayer.h>
#include <WebCore/TextureMapperPlatformLayerProxy.h>
#include <wtf/Function.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/RunLoop.h>
#include <wtf/ThreadingPrimitives.h>
#include <wtf/Vector.h>

namespace WebCore {
class CoordinatedBackingStore;
}

namespace WebKit {

class CoordinatedGraphicsSceneClient {
public:
    virtual ~CoordinatedGraphicsSceneClient() { }
    virtual void updateViewport() = 0;
#if ENABLE(DAMAGE_TRACKING)
    virtual const WebCore::Damage& addSurfaceDamage(const WebCore::Damage&) = 0;
#endif
};

class CoordinatedGraphicsScene : public ThreadSafeRefCounted<CoordinatedGraphicsScene>, public WebCore::TextureMapperPlatformLayerProxy::Compositor {
public:
    explicit CoordinatedGraphicsScene(CoordinatedGraphicsSceneClient*);
    virtual ~CoordinatedGraphicsScene();

    void applyStateChanges(const Vector<RefPtr<Nicosia::Scene>>&);
    void paintToCurrentGLContext(const WebCore::TransformationMatrix&, const WebCore::FloatRect&, bool flipY = false);
    void updateSceneState();
    void detach();

    // The painting thread must lock the main thread to use below two methods, because two methods access members that the main thread manages. See m_client.
    // Currently, QQuickWebPage::updatePaintNode() locks the main thread before calling both methods.
    void purgeGLResources();

    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

#if ENABLE(DAMAGE_TRACKING)
    void setDamagePropagation(WebCore::Damage::Propagation damagePropagation) { m_damagePropagation = damagePropagation; }
#endif

private:
    void commitSceneState(const RefPtr<Nicosia::Scene>&);

    WebCore::TextureMapperLayer* rootLayer() { return m_rootLayer.get(); }

    void removeLayer(Nicosia::CompositionLayer&);

    void updateViewport();

    void ensureRootLayer();

    void onNewBufferAvailable() override;

    struct {
        RefPtr<Nicosia::Scene> scene;
        Nicosia::Scene::State state;
    } m_nicosia;

    std::unique_ptr<WebCore::TextureMapper> m_textureMapper;

    // Below two members are accessed by only the main thread. The painting thread must lock the main thread to access both members.
    CoordinatedGraphicsSceneClient* m_client;
    bool m_isActive { false };

#if ENABLE(DAMAGE_TRACKING)
    WebCore::Damage::Propagation m_damagePropagation { WebCore::Damage::Propagation::None };
#endif

    std::unique_ptr<WebCore::TextureMapperLayer> m_rootLayer;

    Nicosia::PlatformLayer::LayerID m_rootLayerID { 0 };

    HashMap<WebCore::TextureMapperLayer*, Ref<WebCore::CoordinatedBackingStore>> m_backingStores;

    WebCore::TextureMapperFPSCounter m_fpsCounter;
};

} // namespace WebKit

#endif // USE(COORDINATED_GRAPHICS)


