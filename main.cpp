#include <cfloat>

#include "raylib.h"
#include "raymath.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
/*agent one
 *major: like a vacuum cleaner, moves to the area with the most food in its radius
 *every x seconds it picks up everything in it's radius
 *minor: based on the number of food it gets, it has a speed boost kx time
 *
 *agent two
 *major: slower than other agents
 *follower agent that follows the closest other agent near it, picks up as much as it can within
 *x seconds and then consumes half of it and drops the other half
 *if food comes within very small radius it will deviate from path and collect
 *minor: maxforce increases or decreases based on food density
 */

using namespace std;
class Target
{
public:
    Vector2 position;
    bool collected;
    float radius;
    bool isHigh;
    Target(){position={0,0}; collected=false; radius=1.0f;}
    Target(Vector2 p)
    {
        position = p;
        collected = false;
        radius = 7.0;
        int x=rand()%2;
        if(x==1)
        {
            isHigh=true;
        }
        else{isHigh=false;}

    }

    Vector2 getPosition()
    {
        return position;
    }

    void draw()
    {
        if (!collected)
        {
            DrawCircle(position.x, position.y, radius, BLACK);
        }
    }
};
class ALavatar
{
public:
    float maxSpeed, maxForce;
    Image image;
    Vector2 position, velocity, acceleration;
    Texture2D texture;
    float rotation;
    float radius;
    float pickupRadius;
    float pickupInterval;
    float lastPickupTime;
    Vector2 wanderTarget;
    float timeToChangeTarget;
    bool isPickingUp, isWandering;
    float pickupStartTime;
    float pickupDuration;
    int collectedTargets;
    bool isBeingFollowed, active;

    ALavatar(float maxS, float maxF, Vector2 pos, Image i)
    {
        maxSpeed = maxS;
        maxForce = maxF;
        image = i;
        position = pos;
        velocity = {0.0f, 0.0f};
        acceleration = {0.0f, 0.0f};
        rotation = 0;
        radius = 30.0f;
        pickupRadius = 80.0f;
        pickupInterval = 5.0f;
        lastPickupTime=0.0f;
        wanderTarget = pos;
        isPickingUp = false;
        isWandering = false;
        isBeingFollowed = false;
        active = true;
        collectedTargets = 0;
        pickupDuration = 2.0f;
        pickupStartTime = 0.0f;
        timeToChangeTarget = 0.0f;
        ImageResize(&image, 60, 60);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    Vector2 GetPosition()
    {
        return position;
    }

    void update(Vector2 target, float deltaTime)
    {
        if (!active) return;
        Vector2 desired = Vector2Subtract(target, position);
        float distance = Vector2Length(desired);
        desired = Vector2Normalize(desired);
        if (distance < 100.0f)
        {
            float scaledSpeed = maxSpeed * (distance / 100.0f);
            desired = Vector2Scale(desired, scaledSpeed);
        }
        else
        {
            desired = Vector2Scale(desired, maxSpeed);
        }
        acceleration = Vector2Subtract(desired, velocity);
        acceleration = Vector2Scale(Vector2Normalize(acceleration), maxForce);
        velocity = Vector2Add(velocity, Vector2Scale(acceleration, deltaTime));
        position = Vector2Add(position, Vector2Scale(velocity, deltaTime));
        if (Vector2Length(velocity) > 0)
        {
            rotation = atan2f(velocity.y, velocity.x) * (180.0f / PI);
        }
    }

    void moveTowardsBestTargetArea(Target targets[], int targetCount, float deltaTime, float currentTime)
    {
        Vector2 bestTarget = findBestTargetArea(targets, targetCount);
        update(bestTarget, deltaTime);
        pickUpTargets(targets, targetCount, currentTime);
    }
    Vector2 findBestTargetArea(Target targets[], int targetCount)
    {
        int maxTargetsInRadius = 0;
        Vector2 bestArea=position;
        int tInRadius = 0;

        for(int i=0; i<targetCount; i++)
        {
            tInRadius = 0;
            Vector2 sumPos = {0,0};
            if(!targets[i].collected)
            {
                if(Vector2Distance(targets[i].position, position) <pickupRadius)//prioritizes targets within pickup radius
                {
                    for(int k=0; k<targetCount; k++)
                    {
                        if(!targets[k].collected && Vector2Distance(targets[i].position, targets[k].position) < pickupRadius)
                        {
                            tInRadius++;
                            sumPos = Vector2Add(sumPos, targets[k].position);
                        }
                        if (tInRadius > maxTargetsInRadius)
                        {
                            maxTargetsInRadius = tInRadius;
                            bestArea = Vector2Scale(sumPos, 1.0f / tInRadius);
                        }                    }
                }
                else
                {
                    for(int j=0; j<targetCount; j++)
                    {
                        if(!targets[j].collected && Vector2Distance(targets[i].position, targets[j].position) < pickupRadius)
                        {
                            tInRadius++;
                            sumPos = Vector2Add(sumPos, targets[j].position);
                        }
                    }

                    if(tInRadius>maxTargetsInRadius)
                    {
                        maxTargetsInRadius = tInRadius;
                        bestArea = Vector2Scale(sumPos, 1.0f / maxTargetsInRadius);
                    }
                }
            }
        }
        return bestArea;
    }
    void wander(float deltaTime, int width, int height)
    {
        float wanderRadius = pickupRadius * 2;
        float distanceToTarget = Vector2Distance(position, wanderTarget);
        float changeInterval = distanceToTarget / (Vector2Length(velocity) + 1.0f);//making it dependent on speed so it doesn't get to the target too quickly
        timeToChangeTarget += deltaTime;
        if (timeToChangeTarget >= changeInterval)
        {
            timeToChangeTarget = 0.0f;
            float x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f; //val from -1 to 1
            float y = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            Vector2 randomDirection = Vector2Normalize(Vector2{x, y});
            wanderTarget = Vector2Add(position, Vector2Scale(randomDirection, wanderRadius));
            if (wanderTarget.x < 0.0f)
                wanderTarget.x = 0.0f;
            else if (wanderTarget.x > width)
                wanderTarget.x = width;

            if (wanderTarget.y < 0.0f)
                wanderTarget.y = 0.0f;
            else if (wanderTarget.y > height)
                wanderTarget.y = height;
        }

        update(wanderTarget, deltaTime);
    }

    void deactivate() {
        active = false;
    }
    void move(Target targets[], int targetCount, float deltaTime, float currentTime, int width, int height)//main method that needs to be called
    {
        int targetsInBestArea = 0;
        for (int i = 0; i < targetCount; i++)
        {
            if (!targets[i].collected && Vector2Distance(position, targets[i].position) <= pickupRadius)
            {
                targetsInBestArea++;
            }
        }
        if (targetsInBestArea > 1)
        {
            Vector2 bestTargetArea = findBestTargetArea(targets, targetCount);
            update(bestTargetArea, deltaTime);
            isWandering=false;
        }
        else
        {
            wander(deltaTime, width, height);
            isWandering = true;
        }
        pickUpTargets(targets, targetCount, currentTime);
    }
    bool targetInRadius(Target targets[], int targetCount)
    {
        for (int i = 0; i < targetCount; i++)
        {
            if(!targets[i].collected)
            {
                if(Vector2Distance(position, targets[i].position) < pickupRadius)
                {
                    return true;
                }
            }
        }
        return false;
    }
    void pickUpTargets(Target targets[], int targetCount, float currentTime)
    {
        if (!isPickingUp && currentTime - lastPickupTime >= pickupInterval)
        {
            isPickingUp = true;
            pickupStartTime = currentTime;
            lastPickupTime = currentTime;
        }
        if (isPickingUp)
        {
            if (currentTime - pickupStartTime <= pickupDuration)
            {
                for (int i = 0; i < targetCount; i++)
                {
                    if (!targets[i].collected && Vector2Distance(position, targets[i].position) <= pickupRadius)
                    {
                        targets[i].collected = true;
                        collectedTargets++;
                        if(collectedTargets%7==0)
                        {
                            pickupRadius+=20.0f;
                        }
                    }
                }
            }
            else
            {
                isPickingUp = false;
            }
        }
    }
    void evolve() {
        int x = rand()*2;
        if(x==1)
        {
            maxSpeed += 20.0f * collectedTargets;
        }
        else{maxForce += 10.0f * collectedTargets;}
        pickupRadius = 80.0f;
        collectedTargets = 0;
    }
    void draw()
    {
        if (!active) return;

        Color circleColor = BLUE;;

        if (isPickingUp)
        {
            circleColor = GREEN;
        }
        else if (isWandering)
        {
            circleColor = RED;
        }
        else
        {
            circleColor = BLUE;
        }
        DrawCircle(position.x, position.y, pickupRadius, Fade(circleColor, 0.3f));
        Rectangle sourceRect = {0.0f, 0.0f, (float)texture.width, (float)texture.height};
        Rectangle destRect = {position.x, position.y, (float)texture.width, (float)texture.height};
        Vector2 origin = {(float)texture.width / 2.0f, (float)texture.height / 2.0f};
        DrawTexturePro(texture, sourceRect, destRect, origin, rotation + 90, WHITE);
        std::string str = std::to_string(collectedTargets);
        DrawText(str.c_str(), position.x+30, position.y-25, 20, BLACK);
    }

    bool checkCollision(Vector2 targetPos, float targetRadius)
    {
        float distance = Vector2Distance(position, targetPos);
        return distance <= (radius + targetRadius);
    }
};
class RSavatar {
public:
    Vector2 position;
    Vector2 velocity;
    float maxSpeed;
    float maxForce;
    Texture2D texture;
    int score;
    float minPickupRadius;
    Vector2 points[10];
    Vector2 highPoints[5];
    float totalTime;
    float dumpDisplayTime;
    bool hasDumped, active;
    Vector2 wanderTarget;
    float timeToChangeTarget;
    int previousIndex;
    int width, height;
    bool following;


public:
    RSavatar(Vector2 startPosition, Texture2D carTexture, int w, int h) {
        position = startPosition;
        velocity = {0, 0};
        maxSpeed = 300.0f;
        maxForce = 200.0f;
        score = 0;
        width = w;
        height = h;
        active=true;
        wanderTarget = startPosition;
        texture = carTexture;
        minPickupRadius = 65.0f;
        totalTime = 0.0f;
        dumpDisplayTime = 0.0f;
        hasDumped = false;
        timeToChangeTarget=0.0f;
        previousIndex=-1;
        following=false;



        for (int i = 0; i < 10; i++) {
            points[i] = { (float)GetRandomValue(0, GetScreenWidth() - 100), (float)GetRandomValue(0, GetScreenHeight() - 100) };
        }

        for (int i = 0; i < 5; i++) {
            highPoints[i] = { (float)GetRandomValue(0, GetScreenWidth() - 100), (float)GetRandomValue(0, GetScreenHeight() - 100) };
        }
    }
    void deactivate() {
        active = false;
    }
    void goToTarget(Vector2 target, float deltaTime)
    {
        if (!active) return;
        Vector2 desired = Vector2Subtract(target, position);
        float distance = Vector2Length(desired);
        desired = Vector2Normalize(desired);
        if (distance < 100.0f)
        {
            float scaledSpeed = maxSpeed * (distance / 100.0f);
            desired = Vector2Scale(desired, scaledSpeed);
        }
        else
        {
            desired = Vector2Scale(desired, maxSpeed);
        }
        Vector2 acceleration = Vector2Subtract(desired, velocity);
        acceleration = Vector2Scale(Vector2Normalize(acceleration), maxForce);
        velocity = Vector2Add(velocity, Vector2Scale(acceleration, deltaTime));
        position = Vector2Add(position, Vector2Scale(velocity, deltaTime));
    }
    void movetwo(ALavatar avatars[], float deltaTime, int avatarsCount, Target targets[], int targetCount, int width, int height) {
        Vector2 targetPos;
        bool goingToTarget = false;
        int closestTargetIndex = -1;
        int closestAvatarIndex = -1;
        float closestTargetDistance = FLT_MAX;
        float closestAvatarDistance = FLT_MAX;
        float minFollowingRadius = 100.0f;

        for (int i = 0; i < targetCount; i++) {
            if (!targets[i].collected) {
                float distanceToTarget = Vector2Distance(position, targets[i].position);
                if (distanceToTarget < minPickupRadius) {
                    closestTargetDistance = distanceToTarget;
                    closestTargetIndex = i;
                }
            }
        }

        if (closestTargetIndex != -1) {
            if (previousIndex != -1) {
                avatars[previousIndex].isBeingFollowed = false;
                previousIndex = -1;
            }

            targetPos = targets[closestTargetIndex].position;
            goingToTarget = true;
            following = false;

        }

        if (!goingToTarget) {
            for (int i = 0; i < avatarsCount; i++) {
                if (!avatars[i].isBeingFollowed && avatars[i].active) {
                    float distanceToAvatar = Vector2Distance(position, avatars[i].position);
                    if (distanceToAvatar < minFollowingRadius && distanceToAvatar < closestAvatarDistance) {
                        closestAvatarDistance = distanceToAvatar;
                        closestAvatarIndex = i;
                    }
                }
            }

            if (closestAvatarIndex != -1) {
                if (previousIndex != -1 && previousIndex != closestAvatarIndex) {
                    avatars[previousIndex].isBeingFollowed = false;
                }

                avatars[closestAvatarIndex].isBeingFollowed = true;
                previousIndex = closestAvatarIndex;

                targetPos = avatars[closestAvatarIndex].position;
                goingToTarget = true;
                following =true;
            }
        }

        if (goingToTarget) {
            goToTarget(targetPos, deltaTime);
        } else {
            if (previousIndex != -1) {
                avatars[previousIndex].isBeingFollowed = false;
                previousIndex = -1;
            }
            wander(deltaTime, width, height);
        }
    }


    void Update(ALavatar avatars[], int avatarCount, Target targets[], int targetCount) {
        if (!active) return;
        float changeInTime = GetFrameTime();
        totalTime += changeInTime;

        movetwo(avatars, changeInTime, avatarCount, targets, targetCount, width, height);

        for (int i = 0; i < targetCount; i++) {
            if (!targets[i].collected && Vector2Distance(position, targets[i].position) <= 20.0f) {
                targets[i].collected = true;
                score++;
            }
        }

        /*if ((int)(totalTime) % 15 == 0 && !hasDumped) {
            int pointsToDump = score / 2;
            score /= 2;
            hasDumped = true;
            dumpDisplayTime = 5.0f;
            for (int i = 0; i < pointsToDump; i++) {
                points[i].x = GetRandomValue(0, GetScreenWidth() - 100);
                points[i].y = GetRandomValue(0, GetScreenHeight() - 100);
            }
        }*/

        if ((int)(totalTime) % 15 != 0) {
            hasDumped = false;
        }

        if (dumpDisplayTime > 0) {
            dumpDisplayTime -= changeInTime;
        }
    }
    void evolve() {
        maxSpeed += 20.0f*score;
        maxForce += 10.0f*score;
        score=0;
    }
    void wander(float deltaTime, int width, int height)
    {
        float wanderRadius = 80.0f;
        float distanceToTarget = Vector2Distance(position, wanderTarget);
        float changeInterval = distanceToTarget / (Vector2Length(velocity) + 1.0f);
        timeToChangeTarget += deltaTime;

        if (timeToChangeTarget >= changeInterval)
        {
            timeToChangeTarget = 0.0f;

            float x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f; // Value from -1 to 1
            float y = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            Vector2 randomDirection = Vector2Normalize(Vector2{x, y});

            wanderTarget = Vector2Add(position, Vector2Scale(randomDirection, wanderRadius));

            if (wanderTarget.x < 0.0f) wanderTarget.x = 0.0f;
            else if (wanderTarget.x > width) wanderTarget.x = width;

            if (wanderTarget.y < 0.0f) wanderTarget.y = 0.0f;
            else if (wanderTarget.y > height) wanderTarget.y = height;
        }

        Vector2 desired = Vector2Subtract(wanderTarget, position);
        float distance = Vector2Length(desired);
        desired = Vector2Normalize(desired);

        if (distance < 100.0f)
        {
            float scaledSpeed = maxSpeed * (distance / 100.0f);
            desired = Vector2Scale(desired, scaledSpeed);
        }
        else
        {
            desired = Vector2Scale(desired, maxSpeed);
        }

        Vector2 acceleration = Vector2Subtract(desired, velocity);
        acceleration = Vector2Scale(Vector2Normalize(acceleration), maxForce);
        velocity = Vector2Add(velocity, Vector2Scale(acceleration, deltaTime));
        velocity = Vector2ClampValue(velocity, 0, maxSpeed); // Clamp speed

        position = Vector2Add(position, Vector2Scale(velocity, deltaTime));
    }



    void Draw() {
        if (!active) return;
        Rectangle sourceRect = {0.0f, 0.0f, (float)texture.width, (float)texture.height};
        Rectangle destRect = {position.x, position.y, (float)texture.width, (float)texture.height};
        Vector2 origin = {(float)texture.width / 2.0f, (float)texture.height / 2.0f};
        float angle = atan2(velocity.y, velocity.x) * (180.0f / PI);
        DrawTexturePro(texture, sourceRect, destRect, origin, angle + 90, WHITE);

        std::string str = std::to_string(score);

        if(!following)
        {
            DrawText(str.c_str(), position.x+30, position.y-25, 20, RED);
        }
        else
        {
            DrawText(str.c_str(), position.x+30, position.y-25, 20, BLACK);
        }


        if (totalTime > 10 && dumpDisplayTime > 0) {
            //DrawText("Half the follower agents' points have been dumped!", 10, 70, 5.0f, BLACK);
        }
    }

    bool ShouldClose() {
        return WindowShouldClose();
    }

    Vector2 GetPosition() {
        return position;
    }
};




int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    srand(static_cast<unsigned int>(time(0)));
    int width =1300;
    int height = 800;
    InitWindow(width, height, "Automation Game");
    SetTargetFPS(60);
    Image player = LoadImage("assets/spaceship2.png");
    ImageResize(&player, 50, 50);
    Texture2D car = LoadTextureFromImage(player);
    float startTime=0.0f;

    ALavatar avatars[6]={
            ALavatar(100.0f, 100.0f, Vector2{400, 225}, LoadImage("assets/spaceship3.png")),
            ALavatar(100.0f, 100.0f, Vector2{400, 625}, LoadImage("assets/spaceship3.png")),
            ALavatar(100.0f, 100.0f, Vector2{400, 125}, LoadImage("assets/spaceship3.png")),
            ALavatar(100.0f, 100.0f, Vector2{400, 325}, LoadImage("assets/spaceship3.png")),
            ALavatar(100.0f, 100.0f, Vector2{400, 425}, LoadImage("assets/spaceship3.png")),
            ALavatar(100.0f, 100.0f, Vector2{400, 525}, LoadImage("assets/spaceship3.png"))

    };
    RSavatar agents[5]={
            RSavatar( Vector2{100, 225},car, width, height),
            RSavatar(Vector2{200, 225},car, width, height),
            RSavatar(Vector2{300, 225},car, width, height),
            RSavatar(Vector2{400, 225},car, width, height),
            RSavatar(Vector2{500, 225},car, width, height)
    };
    int avatarsCount = sizeof(avatars) / sizeof(avatars[0]);
    Target targets[120];
    int targetCount= sizeof(targets)/sizeof(targets[0]);
    for (int i = 0; i < targetCount; i++)
    {
        float x = rand() % width;
        float y = rand() % height;
        targets[i] = Target(Vector2{x, y});
    }
    float trigger = 0.0f;
    float current = 0.0f;
    float eliminationInterval = 30.0f;
    float lastEliminationTime = 0.0f;
    bool change=false;
    while (!WindowShouldClose())
    {
        int elapsedTime = static_cast<int>(floor(GetTime() - startTime));

        for(RSavatar& avatar: agents)
        {
            avatar.Update(avatars, avatarsCount, targets, targetCount);
        }
        float deltaTime = GetFrameTime();
        current = GetTime();
        if (current - trigger >= 30.0f)
        {
            for (Target& target : targets)
            {
                float x = rand() % width;
                float y = rand() % height;
                target.position = Vector2{x, y};
                target.collected = false;
            }
            trigger = current;
            change = !change;

        }
        for(ALavatar& avatar: avatars)
        {
            avatar.move(targets,targetCount, deltaTime, current, width, height);
        }
        if (current - lastEliminationTime >= eliminationInterval) {
            bool fromAL;

            int lowestScore = INT_MAX;
            int avatarToDeactivate = -1;
            for (int i = 0; i < avatarsCount; i++) {
                if (avatars[i].active && avatars[i].collectedTargets < lowestScore) {
                    lowestScore = avatars[i].collectedTargets;
                    avatarToDeactivate = i;
                    fromAL = true;

                }
            }
            int agentToDeactivate = -1;
            for (int i = 0; i < avatarsCount; i++) {
                if (agents[i].active && agents[i].score < lowestScore) {
                    lowestScore = agents[i].score;
                    avatarToDeactivate = i;
                    fromAL = false;
                }
            }
            if (avatarToDeactivate != -1) {
                if(fromAL)
                    avatars[avatarToDeactivate].deactivate();
                else{agents[avatarToDeactivate].deactivate();}
            }

            lowestScore = INT_MAX;
            avatarToDeactivate = -1;
            for (int i = 0; i < avatarsCount; i++) {
                if (avatars[i].active && avatars[i].collectedTargets < lowestScore) {
                    lowestScore = avatars[i].collectedTargets;
                    avatarToDeactivate = i;
                    fromAL = true;

                }
            }
            for (int i = 0; i < avatarsCount; i++) {
                if (agents[i].active && agents[i].score < lowestScore) {
                    lowestScore = agents[i].score;
                    avatarToDeactivate = i;
                    fromAL = false;
                }
            }
            if (avatarToDeactivate != -1) {
                if(fromAL)
                    avatars[avatarToDeactivate].deactivate();
                else{agents[avatarToDeactivate].deactivate();}
            }

            for (int i = 0; i < avatarsCount; i++) {
                if (avatars[i].active) {
                    avatars[i].evolve();
                }
            }
            for (int i = 0; i < avatarsCount; i++) {
                if (agents[i].active) {
                    agents[i].evolve();
                }
            }

            lastEliminationTime = current;
        }int activeAvatars = 0, activeAgents = 0;
        for (int i = 0; i < avatarsCount; i++) {
            if (avatars[i].active) activeAvatars++;
        }
        for (int i = 0; i < avatarsCount; i++) {
            if (agents[i].active) activeAgents++;
        }

        std::string elapsedTimeStr = "Elapsed Time: " + std::to_string(elapsedTime) + "s";

        BeginDrawing();
        DrawText(elapsedTimeStr.c_str(), 10, 10, 20, BLACK);
        if(!change)
        {
            ClearBackground(SKYBLUE);
        }
        else
        {
            ClearBackground(LIGHTGRAY);
        }
        if (activeAvatars +activeAgents==1) {
            DrawText("GAME OVER", width / 2 - 50, height / 2, 20, BLACK);
        }
        else
        {
            for (Target& target : targets)
            {
                target.draw();
            }
            for (ALavatar& avatar : avatars)
            {
                avatar.draw();
            }
            for (RSavatar& game : agents)
            {
                game.Draw();
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}

