# include <Siv3D.hpp>

struct EmojiPolygon
{
	Texture texture;
	MultiPolygon polygons;
};

/// @brief パンのクラスです。
struct Bread
{
	/// @brief パンのボディです。
	P2Body body;

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
	const P2Body stage = world.createPolygon(P2Static, Vec2{ 0,0 }, Polygon{ Vec2{-300,0},Vec2{-300,-500},Vec2{-270,-500},Vec2{-270,-30}, Vec2{270,-30}, Vec2{270,-500}, Vec2{300,-500},Vec2{300,0}});

	// ステージのボディID
	const P2BodyID stageID = stage.id();

	// パンのハッシュテーブル[ボディID, パン]
	HashTable<P2BodyID, Bread> breads;

	// パンの落下開始位置
	Vec2 dropPos{ 0,-500 };

	// パンの落下開始位置の左右移動速度
	double speed = 200.0;

	// パンの落下クールタイム
	double dropCoolTime = 1.0;

	// パンの落下タイマー
	double dropTimer = 0.0;

	// BGMを読み込む
	const Audio playBGM{ U"music/bgm/suikaGameBGM.mp3" };

	// カメラの表示倍率です
	double cameraScale = 1.0;

	// 2D カメラ
	Camera2D camera{ Vec2{ 0, -300 }, cameraScale, CameraControl::None_ };

	// テクスチャとポリゴンの配列
	Array<EmojiPolygon> emojiPolygons;

	// テクスチャとポリゴンを登録
	emojiPolygons << EmojiPolygon{ Texture{ U"🍞"_emoji }, Emoji::CreateImage(U"🍞").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥐"_emoji }, Emoji::CreateImage(U"🥐").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥖"_emoji }, Emoji::CreateImage(U"🥖").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥨"_emoji }, Emoji::CreateImage(U"🥨").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥯"_emoji }, Emoji::CreateImage(U"🥯").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥪"_emoji }, Emoji::CreateImage(U"🥪").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥙"_emoji }, Emoji::CreateImage(U"🥙").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥚"_emoji }, Emoji::CreateImage(U"🥚").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🍳"_emoji }, Emoji::CreateImage(U"🍳").alphaToPolygonsCentered().simplified(2.0) };
	emojiPolygons << EmojiPolygon{ Texture{ U"🥘"_emoji }, Emoji::CreateImage(U"🥘").alphaToPolygonsCentered().simplified(2.0) };

	// メインループ
	while (System::Update())
	{
		// BGMを再生する
		playBGM.play();

		double deltaTime = Scene::DeltaTime();

		for (accumulatedTime += deltaTime; StepTime <= accumulatedTime; accumulatedTime -= StepTime)
		{
			// 2D 物理演算のワールドを更新する
			world.update(StepTime);
		}

		// 地面より下に落ちた物体は削除する
		for (auto& bread : breads)
		{
			if(200 < bread.second.body.getPos().y)
			{
				breads.erase(bread.first);
			}
		}

		// 落下開始位置を右に移動します
		if (KeyRight.pressed() || KeyD.pressed())
		{
			dropPos.x += speed * deltaTime;
		}

		// 落下開始位置を左に移動します
		if (KeyLeft.pressed() || KeyA.pressed())
		{
			dropPos.x -= speed * deltaTime;
		}

		// 落下開始位置の移動範囲を制限します
		dropPos.x = Clamp(dropPos.x, -200.0, 200.0);

		dropTimer += deltaTime;
		// スペースを押したら
		if (KeySpace.down() && dropCoolTime <= dropTimer)
		{
			dropTimer = 0.0;
			// パンを生成
			int32 randomLevel = Random(0, 2);
			P2Body body = world.createPolygons(P2Dynamic, dropPos, emojiPolygons[randomLevel].polygons, P2Material{.density = 0.1});
			breads.emplace(body.id(), Bread{ body,randomLevel });
		}

		// 同じパン同士が衝突したら削除します
		for (auto&& [pair, collision] : world.getCollisions())
		{
			// 衝突したペアがパン同士でなければ次のペアへ
			if (not breads.contains(pair.a) || not breads.contains(pair.b))
			{
				continue;
			}

			// パンのレベルが同じならば
			if (breads[pair.a].level == breads[pair.b].level)
			{
				int32 newBreadLevel = breads[pair.a].level + 1;
				Vec2 newBreadPos = (breads[pair.a].body.getPos() + breads[pair.b].body.getPos()) * 0.5;

				// パンのレベルが最大の時は削除だけする
				if (newBreadLevel != emojiPolygons.size())
				{
					// 新しいパンを生成する
					P2Body body = world.createPolygons(P2Dynamic, newBreadPos, emojiPolygons[newBreadLevel].polygons, P2Material{ .density = 0.1 });
					breads.emplace(body.id(), Bread{ body,newBreadLevel });
				}

				// 衝突したパンを削除する
				breads.erase(pair.a);
				breads.erase(pair.b);
			}
		}
		

		//// --------------------描画処理-------------------------

		Rect{0,0,1280,720 }.draw(Arg::top = Palette::Floralwhite,Arg::bottom = Palette::Navajowhite);

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
				emojiPolygons[bread.second.level].texture.rotated(bread.second.body.getAngle()).drawAt(bread.second.body.getPos());
			}

			// ステージを描画する
			stage.draw(Palette::Black);
		}
		// 2D カメラの操作を描画する
		camera.draw(Palette::Orange);

		
	}
}
