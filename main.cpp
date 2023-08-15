#include <Novice.h>
#define _USE_MATH_DEFINES
#include <imgui.h>
#include "math.h"
#include <algorithm>

#include "Mt4.h"

const char kWindowTitle[] = "LE2C_13_サイトウ_ユウキ_MT3_02_05";

struct AABB
{
	Vector3 min; //!< 最小点
	Vector3 max; //!< 最大点
};

//Grid
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

Vector3 TransformCoord(Vector3 vector, Matrix4x4 matrix);

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	float kWindowWidth = 1280.0f;
	float kWindowHeight = 720.0f;

	Vector3 cameraTranslate{ 0.0f,1.9f,-6.49f };
	Vector3 cameraRotate{ 0.26f,0.0f,0.0f };

	AABB aabb1{
		.min{-0.5f, -0.5f, -0.5f},
		.max{ 0.0f, 0.0f, 0.0f}
	};
	AABB aabb2{
		.min{0.2f, 0.2f, 0.2f},
		.max{ 1.0f, 1.0f, 1.0f}
	};

	uint32_t color = WHITE;

	uint32_t color2 = WHITE;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewPortMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		if (IsCollision(aabb1, aabb2)) {
			color = RED;
		}
		else {
			color = WHITE;
		}

		ImGui::Begin("window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("AABB1min", &aabb1.min.x, 0.1f);
		ImGui::DragFloat3("AABB1max", &aabb1.max.x, 0.1f);
		ImGui::DragFloat3("AABB2min", &aabb2.min.x, 0.1f);
		ImGui::DragFloat3("AABB2max", &aabb2.max.x, 0.1f);
		ImGui::End();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewPortMatrix);

		DrawAABB(aabb1, viewProjectionMatrix, viewPortMatrix, color);
		DrawAABB(aabb2, viewProjectionMatrix, viewPortMatrix, color2);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}

//Grid
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f; //Gridの半分の幅
	const uint32_t kSubdivision = 10; //分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision); //1つ分の長さ
	//奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + (xIndex * kGridEvery);

		//上の情報を使ってワールド座標系上の始点と終点を求める
		Vector3 start = { x,0.0f,-kGridHalfWidth };
		Vector3 end = { x,0.0f,kGridHalfWidth };

		//スクリーン座標系まで変換をかける
		Vector3 screenStart = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

		//変換した座標を使って表示。色は薄い灰色(0xAAAAAAFF),原点は黒ぐらいが良いが、何でも良い
		if (x == 0) {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0x000000FF);
		}
		else {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0xAAAAAAFF);
		}
	}
	//左から右も同じように順々に引いていく
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);

		//上の情報を使ってワールド座標系上の始点と終点を求める
		Vector3 start = { -kGridHalfWidth,0.0f,z };
		Vector3 end = { kGridHalfWidth,0.0f,z };

		//スクリーン座標系まで変換をかける
		Vector3 screenStart = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

		//変換した座標を使って表示。色は薄い灰色(0xAAAAAAFF),原点は黒ぐらいが良いが、何でも良い
		if (z == 0) {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0x000000FF);
		}
		else {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0xAAAAAAFF);
		}
	}
}

Vector3 TransformCoord(Vector3 vector, Matrix4x4 matrix) {
	Vector3 result{};
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];

	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 vers[8]{};
	vers[0] = { aabb.min.x, aabb.min.y, aabb.min.z };
	vers[1] = { aabb.min.x, aabb.min.y, aabb.max.z };
	vers[2] = { aabb.min.x, aabb.max.y, aabb.min.z };
	vers[3] = { aabb.max.x, aabb.min.y, aabb.min.z };
	vers[4] = { aabb.max.x, aabb.max.y, aabb.min.z };
	vers[5] = { aabb.min.x, aabb.max.y, aabb.max.z };
	vers[6] = { aabb.max.x, aabb.min.y, aabb.max.z };
	vers[7] = { aabb.max.x, aabb.max.y, aabb.max.z };

	Vector3 screenVers[8]{};
	for (int i = 0; i < 8; i++) {
		vers[i] = TransformCoord(vers[i], viewProjectionMatrix);
		screenVers[i] = TransformCoord(vers[i], viewportMatrix);
	}

	Novice::DrawLine(int(screenVers[0].x), int(screenVers[0].y), int(screenVers[1].x), int(screenVers[1].y), color);
	Novice::DrawLine(int(screenVers[0].x), int(screenVers[0].y), int(screenVers[2].x), int(screenVers[2].y), color);
	Novice::DrawLine(int(screenVers[0].x), int(screenVers[0].y), int(screenVers[3].x), int(screenVers[3].y), color);

	Novice::DrawLine(int(screenVers[1].x), int(screenVers[1].y), int(screenVers[5].x), int(screenVers[5].y), color);
	Novice::DrawLine(int(screenVers[1].x), int(screenVers[1].y), int(screenVers[6].x), int(screenVers[6].y), color);

	Novice::DrawLine(int(screenVers[2].x), int(screenVers[2].y), int(screenVers[4].x), int(screenVers[4].y), color);
	Novice::DrawLine(int(screenVers[2].x), int(screenVers[2].y), int(screenVers[5].x), int(screenVers[5].y), color);

	Novice::DrawLine(int(screenVers[3].x), int(screenVers[3].y), int(screenVers[4].x), int(screenVers[4].y), color);
	Novice::DrawLine(int(screenVers[3].x), int(screenVers[3].y), int(screenVers[6].x), int(screenVers[6].y), color);

	Novice::DrawLine(int(screenVers[4].x), int(screenVers[4].y), int(screenVers[7].x), int(screenVers[7].y), color);
	Novice::DrawLine(int(screenVers[5].x), int(screenVers[5].y), int(screenVers[7].x), int(screenVers[7].y), color);
	Novice::DrawLine(int(screenVers[6].x), int(screenVers[6].y), int(screenVers[7].x), int(screenVers[7].y), color);

}

bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) &&
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {
		return true;
	}
	else {
		return false;
	}
}