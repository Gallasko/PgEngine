#pragma once

#include <string>
#include "ECS/system.h"
#include "pgconstant.h"

namespace pg
{
    struct Bullet
    {

    };

    enum class BulletPattern { Radial, AtPlayer, Cone };

    struct Weapon
    {
        std::string name = "Unknown";

        float damage = 1.0f;

        float projectileSpeed = 300.0f;
        float projectileLifeTime = 1000.0f;

        float projectileSize = 10.0f;

        BulletPattern pattern = BulletPattern::AtPlayer;

        size_t bulletCount = 1;
        float bulletSpreadAngle = 1.0f;

        float reloadTimeMs = 1000.0f;
        size_t barrelSize = 6;

        int ammo = 18;

        std::vector<constant::Vector2D> fireDirections(const constant::Vector2D& aimDir) const
        {
            std::vector<constant::Vector2D> dirs;

            switch (pattern)
            {
                case BulletPattern::Radial:
                {
                    for (size_t i = 0; i < bulletCount; ++i)
                    {
                        float angle = i * 2 * M_PI / bulletCount;
                        dirs.emplace_back(cos(angle), sin(angle));
                    }

                    break;
                }

                case BulletPattern::AtPlayer:
                {
                    float base = atan2(aimDir.y, aimDir.x);

                    dirs.emplace_back(cos(base), sin(base));

                    break;
                }

                case BulletPattern::Cone:
                {
                    float base = atan2(aimDir.y, aimDir.x);
                    float angle = bulletSpreadAngle;
                    float bCount = bulletCount;

                    if (angle == 0.0f)
                    {
                        LOG_WARNING("Weapon", "Weapon has a 0 bullet spread angle, while shooting in a cone");
                        angle = 1.0f;
                    }

                    if (bCount < 2)
                    {
                        LOG_WARNING("Weapon", "Weapon has less than 2 bullets, while shooting in a cone");
                        bCount = 2;
                    }

                    float half = angle * M_PI/180.f / 2;

                    for (size_t i = 0; i < bCount; ++i)
                    {
                        // Todo divide by 0 error are not catched and make the whole application stall
                        float a = base - half + (2 * half * i / (bCount - 1));

                        dirs.emplace_back(cos(a), sin(a));
                    }

                    break;
                }
            }

            return dirs;
        }
    };

    struct WeaponComponent : public Ctor
    {
        WeaponComponent(const Weapon& w) : weapon(w) {}
        WeaponComponent(const WeaponComponent& rhs) = default;
        WeaponComponent& operator=(const WeaponComponent& rhs) = default;

        virtual void onCreation(EntityRef e)
        {
            ecsRef   = e->world();
            entityId = e->id;
        }

        Weapon   weapon;
        EntitySystem* ecsRef;
        _unique_id    entityId;
};

} // namespace pg
