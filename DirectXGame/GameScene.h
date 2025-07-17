#pragma once

#include <KamataEngine.h>
#include <vector>
#include <string>

class GameScene {
public:
    // コンストラクタ
    GameScene();

    // デストラクタ
    ~GameScene();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

private:
    struct ObjectInstance {
        KamataEngine::WorldTransform worldTransform;
        KamataEngine::Model* model = nullptr;
    };

    std::vector<ObjectInstance> objects_;
    KamataEngine::Camera camera_;
};