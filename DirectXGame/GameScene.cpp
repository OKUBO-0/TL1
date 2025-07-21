#include "GameScene.h"

#include <cassert>  // assert
#include <fstream>  // ifstream
#include <json.hpp> // nlohmann

using namespace KamataEngine;
using json = nlohmann::json;

// コンストラクタ
GameScene::GameScene() {}

// デストラクタ
GameScene::~GameScene() {
	for (auto& object : objects_) {
		delete object.worldTransform;
	}
	objects_.clear();
}

// 初期化処理
void GameScene::Initialize() {
	//--------------------------------------------------
	// レベルデータを格納するための構造体
	//--------------------------------------------------

	// オブジェクトデータ
	struct ObjectData {
		std::string type; // "type"
		std::string name; // "name"

		struct Transform {
			Vector3 translation; // "translation"
			Vector3 rotation;    // "rotation"
			Vector3 scaling;     // "scaling"
		} transform;             // "transform"

		std::string file_name; // "file_name"
	};

	// レベルデータ
	struct LevelData {
		std::string name;                // "name"
		std::vector<ObjectData> objects; // "objects"
	};

	//--------------------------------------------------
	// jsonファイルの デシリアライズ化
	//--------------------------------------------------

	// レベルデータのファイルパス
	const std::string fullpath = std::string("Resources/levels/") + "scene.json";

	// ファイルを開く
	std::ifstream file(fullpath);

	// ファイルオープン失敗をチェック
	assert(!file.fail());

	json deserialized;
	file >> deserialized;

	assert(deserialized.is_object());         // object か ※ json形式には様々な型がある
	assert(deserialized.contains("name"));    // "name" が含まれているか
	assert(deserialized["name"].is_string()); // ["name"]は 文字列か?

	//--------------------------------------------------
	// レベルデータを構造体に格納していく
	//--------------------------------------------------

	LevelData* levelData = new LevelData();

	// "name"を文字列として格納
	levelData->name = deserialized["name"].get<std::string>();
	assert(levelData->name == "scene"); // それは "scene" か?

	// "objects"の全オブジェクトを走査
	for (json& object : deserialized["objects"]) {
		// オブジェクト 1つ分の妥当性のチェック
		assert(object.contains("type"));

		if (object.contains("disabled")) {
			// 有効無効フラグ
			bool disabled = object["disabled"].get<bool>();
			if (disabled) {
				// 配置しない (スキップ)
				continue;
			}
		}

		if (object["type"].get<std::string>() == "MESH") {
			// 1個分の要素の準備
			levelData->objects.emplace_back(ObjectData{});
			ObjectData& objectData = levelData->objects.back(); // 追加要素の参照を用意し可読性も良くなる

			objectData.type = object["type"].get<std::string>(); // "type"
			objectData.name = object["name"].get<std::string>(); // "name"

			// 平行移動 "translation"
			auto& transform = object["transform"];
			objectData.transform.translation = {(float)transform["translation"][0], (float)transform["translation"][2], (float)transform["translation"][1]};

			// 回転角   "rotation"
			objectData.transform.rotation = {(float)transform["rotation"][0], (float)transform["rotation"][2], (float)transform["rotation"][1]};

			// 拡大縮小 "scaling"
			objectData.transform.scaling = {(float)transform["scaling"][0], (float)transform["scaling"][2], (float)transform["scaling"][1]};

			// ファイル名 "file_name"
			if (object.contains("file_name")) {
				objectData.file_name = object["file_name"].get<std::string>();
			}
		}
	}

	//----------------------------------------------------
	// レベルデータからオブジェクトを生成、配置
	//----------------------------------------------------
	for (auto& objectData : levelData->objects) {
		// モデルファイル名 objectData.file_name にあれば入ってくる ⇒ file_name を元に、モデルデータを特定する
		if (objectData.file_name.empty())
			continue;

		camera.Initialize();

		// ワールド変換を初期化
		WorldTransform* worldTransform = new WorldTransform();
		worldTransform->Initialize();

		// 位置の設定 objectData.transform.translation に入っている
		worldTransform->translation_ = objectData.transform.translation;

		// 回転の設定 objectData.transform.rotation に入っている
		worldTransform->rotation_ = objectData.transform.rotation;

		// 拡大縮小の設定 objectData.transform.scaling に入っている
		worldTransform->scale_ = objectData.transform.scaling;

		// モデル読み込み
		Model* model = nullptr;
		if (models.contains(objectData.file_name)) {
			model = models[objectData.file_name];
		} else {
			model = Model::CreateFromOBJ(objectData.file_name.c_str());
			models[objectData.file_name] = model;
		}

		// objects_ に登録
		objects_.emplace_back(ObjectInstance{worldTransform, model});
	}
	delete levelData;
}

// 更新処理
void GameScene::Update() {
	for (auto& object : objects_) {
		object.worldTransform->UpdateMatrix();
	}
}

// 描画処理
void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	for (auto& object : objects_) {
		object.model->Draw(*object.worldTransform, camera);
	}

	Model::PostDraw();
}