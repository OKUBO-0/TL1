#include "GameScene.h"

#include <cassert>  // assert
#include <fstream>  // ifstream
#include <json.hpp> // nlohmann
#include <string>   // string
#include <vector>   // vector

using namespace KamataEngine;

// コンストラクタ
GameScene::GameScene() {
}

// デストラクタ
GameScene::~GameScene() {
	 // 登録したモデルを解放
    for (auto& obj : objects_) {
        delete obj.model;
    }
}

void GameScene::Initialize() {
	//--------------------------------------------------
	// レベルデータを格納するための構造体
	//--------------------------------------------------

	// オブジェクト 1個分のデータ
	struct ObjectData {
		std::string type; // "type"
		std::string name; // "name"

		// "transform"
		struct Transform {
			Vector3 translation; // "translation"
			Vector3 rotation;    // "rotation"
			Vector3 scaling;     // "scaling"
		};

		Transform transform; // メンバの準備

		// "file_name"
		std::string file_name;
	};

	// レベルデータ
	struct LevelData {
		// "name"
		std::string name;

		// "objects"
		std::vector<ObjectData> objects;
	};

	//--------------------------------------------------
	// jsonファイルの デシリアライズ化
	//--------------------------------------------------

	// jsonファイルのパス名
	const std::string fullpath = std::string("Resources/levels/") + "scene.json";

	// ファイルストリーム
	std::ifstream file;

	// ファイルを開く
	file.open(fullpath);
	// ファイルオープン失敗をチェック
	if (file.fail()) {
		assert(0);
	}

	nlohmann::json deserialized; // deserialized : 逆シリアライズ化
	                             // ⇒ 1つの文字列をメモリ中のデータ構造化すること
	                             // serialize : 一列に並べる操作のこと
	                             // ⇒ 1つの文字列で表現する「直列化」のこと

	// ファイルから読み込み、メモリへ格納
	file >> deserialized;

	// 正しいレベルデータファイルかチェック
	assert(deserialized.is_object()); // object か ※ json形式には様々な型がある
	                                  // object型はその中でも「キーと値のペアを持つ構造」つまり連想配列が扱えるか聞いている

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
	for (nlohmann::json& object : deserialized["objects"]) {

		// オブジェクト 1つ分の妥当性のチェック
		assert(object.contains("type")); // "type" が含まれているか

		if (object["type"].get<std::string>() == "MESH") {

			// 1個分の要素の準備
			levelData->objects.emplace_back(ObjectData{});
			ObjectData& objectData = levelData->objects.back(); // 追加要素の参照を用意し可読性も良くなる

			objectData.type = object["type"].get<std::string>(); // "type"
			objectData.name = object["name"].get<std::string>(); // "name"

			// トランスフォームのパラメータ読み込み
			nlohmann::json& transform = object["transform"];
			// 平行移動 "translation"
			objectData.transform.translation.x = (float)transform["translation"][0];
			objectData.transform.translation.y = (float)transform["translation"][2];
			objectData.transform.translation.z = (float)transform["translation"][1];

			// 回転角   "rotation"
			objectData.transform.rotation.x = (float)transform["rotation"][0];
			objectData.transform.rotation.y = (float)transform["rotation"][2];
			objectData.transform.rotation.z = (float)transform["rotation"][1];

			// 拡大縮小 "scaling"
			objectData.transform.scaling.x = (float)transform["scaling"][0];
			objectData.transform.scaling.y = (float)transform["scaling"][2];
			objectData.transform.scaling.z = (float)transform["scaling"][1];

			// "file_name"
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
		if (objectData.file_name.empty()) {
			continue;
		}

		ObjectInstance objInst;

		// ワールド変換を初期化
		objInst.worldTransform.Initialize();

		// 位置の設定 objectData.transform.translation に入っている
		objInst.worldTransform.translation_ = objectData.transform.translation;

		// 回転の設定 objectData.transform.rotation に入っている
		objInst.worldTransform.rotation_ = objectData.transform.rotation;

		// 拡大縮小の設定 objectData.transform.scaling に入っている
		objInst.worldTransform.scale_ = objectData.transform.scaling;

		// モデル読み込み
		objInst.model = KamataEngine::Model::CreateFromOBJ(objectData.file_name.c_str());

		// objects_ に登録
		objects_.push_back(std::move(objInst));
	}

	delete levelData;
}

void GameScene::Update() {
	for (auto& obj : objects_) {
		obj.worldTransform.UpdateMatrix();
	}
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	for (auto& obj : objects_) {
		obj.model->Draw(obj.worldTransform, camera_);
	}

	Model::PostDraw();
}