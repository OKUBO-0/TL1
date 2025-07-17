#pragma once

#include <KamataEngine.h>
#include <vector>
#include <map>
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
		KamataEngine::WorldTransform* worldTransform;
		KamataEngine::Model* model;
	};

	std::vector<ObjectInstance> objects_;
	std::map<std::string, KamataEngine::Model*> models;

	KamataEngine::Camera camera;
};