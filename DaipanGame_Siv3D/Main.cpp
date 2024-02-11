# include <Siv3D.hpp>

/// @brief パンのクラスです。
struct Bread
{
	/// @brief パンのボディです。
	P2Body body;

	/// @brief パンのIDです。
	P2BodyID id;

	/// @brief パンの種類です。
	int32 level;
};

void Main()
{
	// ウィンドウを 1280x720 にリサイズする
	Window::Resize(1280, 720);

	// 2D 物理演算のシミュレーションステップ（秒）
	constexpr double StepTime = (1.0 / 200.0);

	// 2D 物理演算のシミュレーション蓄積時間（秒）
	double accumulatedTime = 0.0;

	// 重力加速度 (cm/s^2)
	constexpr double Gravity = 980;

	// 2D 物理演算のワールド
	P2World world{ Gravity };

	// ステージのボディ (1辺 400 cm ）
	const P2Body stage = world.createLineString(P2Static, Vec2{ 0, 0 }, { Vec2{-200, -400}, Vec2{-200, 0}, Vec2{200, 0}, {Vec2{200, -400}} });

	// ステージのボディID
	const P2BodyID stageID = stage.id();

	// パンの配列
	Array<Bread> breads;

	// パンの落下開始位置
	Vec2 dropPos{ 0,-500 };

	// パンの落下開始位置の左右移動速度
	double speed = 200.0;

	// カメラの表示倍率です
	double cameraScale = 1.0;

	// 2D カメラ
	Camera2D camera{ Vec2{ 0, -300 }, cameraScale, CameraControl::None_ };

	// テクスチャからポリゴン生成
	const Texture plainBreadTexture{ U"🍞"_emoji };
	const MultiPolygon plainBreadPolygon = Emoji::CreateImage(U"🍞").alphaToPolygonsCentered().simplified(2.0);

	const Texture croissantTexture{ U"🥐"_emoji };
	const MultiPolygon croissantPolygon = Emoji::CreateImage(U"🥐").alphaToPolygonsCentered().simplified(2.0);

	// メインループ
	while (System::Update())
	{
		double deltaTime = Scene::DeltaTime();

		for (accumulatedTime += deltaTime; StepTime <= accumulatedTime; accumulatedTime -= StepTime)
		{
			// 2D 物理演算のワールドを更新する
			world.update(StepTime);
		}

		// 地面より下に落ちた物体は削除する
		breads.remove_if([](const Bread& b) { return (200 < b.body.getPos().y); });

		// 落下開始位置を右に移動します
		if (KeyRight.pressed())
		{
			dropPos.x += speed * deltaTime;
		}

		// 落下開始位置を左に移動します
		if (KeyLeft.pressed())
		{
			dropPos.x -= speed * deltaTime;
		}

		// 落下開始位置の移動範囲を制限します
		dropPos.x = Clamp(dropPos.x, -200.0, 200.0);

		// スペースを押したら
		if (KeySpace.down())
		{
			// パンを生成
			P2Body body = world.createPolygons(P2Dynamic, dropPos, plainBreadPolygon, P2Material{ .density = 0.1 });
			breads << Bread{ body ,0 };
		}

		// 同じパン同士が衝突したら削除します
		/*for (auto&& [pair, collision] : world.getCollisions())
		{
			if ()
		}*/

		// --------------------描画処理-------------------------

		// 2D カメラを更新する
		camera.update();
		{
			// 2D カメラから Transformer2D を作成する
			const auto t = camera.createTransformer();

			// 落下開始位置を描画する
			Circle{ dropPos,10 }.draw(Palette::Pink);

			// パンの level に応じて描画する
			for (const auto& bread : breads)
			{
				switch (bread.level)
				{
				case 0:
					plainBreadTexture.rotated(bread.body.getAngle()).drawAt(bread.body.getPos());
					break;

				case 1:
					croissantTexture.rotated(bread.body.getAngle()).drawAt(bread.body.getPos());
					break;

				default:
					break;
				}
			}

			// ステージを描画する
			stage.draw(Palette::Skyblue);
		}

		// 2D カメラの操作を描画する
		camera.draw(Palette::Orange);
	}
}
