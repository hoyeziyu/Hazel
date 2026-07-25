#include "hzpch.h"
#include "Physics2DScene.h"

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"

#include <box2d/box2d.h>

#include <cstring>

namespace Hazel {

	namespace {

		b2WorldId DecodeWorld(uint64_t handle)
		{
			b2WorldId world = b2_nullWorldId;
			if (handle != 0)
				std::memcpy(&world, &handle, sizeof(b2WorldId));
			return world;
		}

		uint64_t EncodeWorld(b2WorldId world)
		{
			uint64_t handle = 0;
			std::memcpy(&handle, &world, sizeof(b2WorldId));
			return handle;
		}

		b2BodyId DecodeBody(uint64_t handle)
		{
			b2BodyId body = b2_nullBodyId;
			if (handle != 0)
				std::memcpy(&body, &handle, sizeof(b2BodyId));
			return body;
		}

		uint64_t EncodeBody(b2BodyId body)
		{
			uint64_t handle = 0;
			std::memcpy(&handle, &body, sizeof(b2BodyId));
			return handle;
		}

		uint64_t EncodeShape(b2ShapeId shape)
		{
			uint64_t handle = 0;
			std::memcpy(&handle, &shape, sizeof(b2ShapeId));
			return handle;
		}

		b2BodyType ToBox2DBodyType(RigidBody2DComponent::Type type)
		{
			switch (type)
			{
			case RigidBody2DComponent::Type::Static: return b2_staticBody;
			case RigidBody2DComponent::Type::Dynamic: return b2_dynamicBody;
			case RigidBody2DComponent::Type::Kinematic: return b2_kinematicBody;
			}
			return b2_staticBody;
		}

	}

	bool Physics2DScene::IsActive(const Scene& scene)
	{
		return scene.m_Physics2DActive;
	}

	void Physics2DScene::Init(Scene& scene)
	{
		if (scene.m_Physics2DActive)
			return;

		auto rbView = scene.m_Registry.view<RigidBody2DComponent>();
		if (rbView.begin() == rbView.end())
			return;

		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.0f, -9.81f };
		b2WorldId world = b2CreateWorld(&worldDef);

		for (auto entity : rbView)
		{
			Entity e = { entity, &scene };
			auto& transform = e.GetComponent<TransformComponent>();
			auto& rb = e.GetComponent<RigidBody2DComponent>();

			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = ToBox2DBodyType(rb.BodyType);
			bodyDef.position = { transform.Translation.x, transform.Translation.y };
			bodyDef.rotation = b2MakeRot(transform.Rotation.z);
			bodyDef.fixedRotation = rb.FixedRotation;
			bodyDef.gravityScale = rb.GravityScale;
			bodyDef.linearDamping = rb.LinearDamping;
			bodyDef.angularDamping = rb.AngularDamping;
			bodyDef.userData = (void*)(uintptr_t)e.GetComponent<IDComponent>().ID;

			b2BodyId bodyId = b2CreateBody(world, &bodyDef);
			rb.RuntimeBodyHandle = EncodeBody(bodyId);
		}

		auto boxView = scene.m_Registry.view<BoxCollider2DComponent>();
		for (auto entity : boxView)
		{
			Entity e = { entity, &scene };
			if (!e.HasComponent<RigidBody2DComponent>())
				continue;

			auto& rb = e.GetComponent<RigidBody2DComponent>();
			b2BodyId bodyId = DecodeBody(rb.RuntimeBodyHandle);
			if (B2_IS_NULL(bodyId))
				continue;

			auto& transform = e.GetComponent<TransformComponent>();
			auto& box = e.GetComponent<BoxCollider2DComponent>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = box.Density;
			shapeDef.material = b2DefaultSurfaceMaterial();
			shapeDef.material.friction = box.Friction;

			b2Polygon polygon = b2MakeOffsetBox(
				transform.Scale.x * box.Size.x,
				transform.Scale.y * box.Size.y,
				{ box.Offset.x, box.Offset.y },
				b2MakeRot(0.0f));

			box.RuntimeShapeHandle = EncodeShape(b2CreatePolygonShape(bodyId, &shapeDef, &polygon));
		}

		scene.m_PhysicsWorldHandle = EncodeWorld(world);
		scene.m_Physics2DActive = true;
	}

	void Physics2DScene::Step(Scene& scene, Timestep ts)
	{
		if (!scene.m_Physics2DActive)
			return;

		b2WorldId world = DecodeWorld(scene.m_PhysicsWorldHandle);
		if (B2_IS_NULL(world))
			return;

		const float fixedTimestep = 0.016f;
		float remaining = ts.GetSeconds();
		while (remaining > 0.0f)
		{
			const float step = remaining > fixedTimestep ? fixedTimestep : remaining;
			b2World_Step(world, step, 4);
			remaining -= step;
		}

		scene.m_Registry.view<RigidBody2DComponent>().each([&](auto entity, auto& rb)
		{
			if (rb.BodyType == RigidBody2DComponent::Type::Static)
				return;

			b2BodyId bodyId = DecodeBody(rb.RuntimeBodyHandle);
			if (B2_IS_NULL(bodyId))
				return;

			Entity e = { entity, &scene };
			auto& transform = e.GetComponent<TransformComponent>();

			const b2Vec2 position = b2Body_GetPosition(bodyId);
			const b2Rot rotation = b2Body_GetRotation(bodyId);
			transform.Translation.x = position.x;
			transform.Translation.y = position.y;
			transform.Rotation.z = b2Rot_GetAngle(rotation);
		});
	}

	void Physics2DScene::Shutdown(Scene& scene)
	{
		if (!scene.m_Physics2DActive)
			return;

		b2WorldId world = DecodeWorld(scene.m_PhysicsWorldHandle);
		if (!B2_IS_NULL(world))
			b2DestroyWorld(world);

		scene.m_PhysicsWorldHandle = 0;
		scene.m_Physics2DActive = false;

		scene.m_Registry.view<RigidBody2DComponent>().each([](auto, auto& rb)
		{
			rb.RuntimeBodyHandle = 0;
		});

		scene.m_Registry.view<BoxCollider2DComponent>().each([](auto, auto& box)
		{
			box.RuntimeShapeHandle = 0;
		});
	}

	void Physics2DScene::ApplyLinearImpulse(RigidBody2DComponent& rigidBody, const glm::vec2& impulse)
	{
		b2BodyId bodyId = DecodeBody(rigidBody.RuntimeBodyHandle);
		if (B2_IS_NULL(bodyId))
			return;

		b2Body_ApplyLinearImpulseToCenter(bodyId, { impulse.x, impulse.y }, true);
	}

}
