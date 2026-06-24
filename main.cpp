#include "overlay.h"
#include "utils.h"
#include <string>

std::string GetLevelName()
{
	char tmpstr[64] = { 0 };
	for (int i = 0; i < 64; i++)
	{
		tmpstr[i] = mem::Read1<char>(GameBase + OFFSET_LEVELNAME + i);
		if (tmpstr[i] == '\0') break;
	}
	return std::string(tmpstr);
}

void setupPlayers()
{
	std::vector<DWORD_PTR> Players;
	Players.reserve(150); // Pre-allocate

	static std::string level_name = GetLevelName();
	int loopsize = (level_name == "mp_rr_canyonlands_staging_mu1" ? 10000 : 70);

	while (true)
	{
		std::vector<DWORD_PTR> TempPlayers;
		for (size_t i = 0; i < loopsize; i++)
		{
			DWORD_PTR Entity = mem::Read1<DWORD_PTR>(GameBase + OFFSET_ENTITYLIST + (i * 0x20));
			if (!Entity) continue;

			std::string signifer = GetSignifier(Entity);
			if (signifer == "player" || signifer == "npc_dummie")
			{
				TempPlayers.push_back(Entity);
			}
		}
		PlayerEntityList = TempPlayers;

		// Increased sleep to reduce CPU usage
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
}

void setupLoot()
{
	while (true)
	{
		std::vector<DWORD_PTR> TempLoots;
		TempLoots.reserve(200);

		DWORD_PTR MyLocalplayer = mem::Read1<DWORD_PTR>(GameBase + OFFSET_LOCAL_ENT);
		Vector Localorigin = mem::Read1<Vector>(MyLocalplayer + OFFSET_ORIGIN);

		for (size_t i = 0; i < 15000; i++)
		{
			DWORD_PTR Entity = mem::Read1<DWORD_PTR>(GameBase + OFFSET_ENTITYLIST + (i * 0x20));
			if (!Entity) continue;

			std::string signifer = GetSignifier(Entity);
			if (signifer == "prop_death_box" || signifer == "prop_survival")
			{
				Vector origin = mem::Read1<Vector>(Entity + OFFSET_ORIGIN);
				if (Localorigin.DistTo(origin) < 50.0f * 50.0f) // Check squared distance for speed
				{
					TempLoots.push_back(Entity);
				}
			}
		}
		EntityLoot = TempLoots;

		// Loot is static; 1-second delay is plenty
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
bool GetVisible(uint64_t LocalPlayerPtr, uint64_t EntityPtr)
{
	float last_visible_time = mem::Read1<float>(EntityPtr + OFFSET_VISIBLE_TIME); // LastVisibleTime
	float current_time = mem::Read1<float>(LocalPlayerPtr + m_currentFramePlayertimeBase); // TimeBase
	bool visible = last_visible_time > current_time - 0.15f;
	return visible;
}

void DrawESP(CheatOverlay* overlay)
{
	// --- 1. Cache globals outside the loop ---
	uint64_t viewRenderer = mem::Read1<uint64_t>(GameBase + OFFSET_RENDER);
	uint64_t viewMatrixPtr = mem::Read1<uint64_t>(viewRenderer + OFFSET_MATRIX);
	Matrix m = mem::Read1<Matrix>(viewMatrixPtr);

	uintptr_t g_locPlayer = mem::Read1<uintptr_t>(GameBase + OFFSET_LOCAL_ENT);
	if (!g_locPlayer) return;

	int LocalTeam = mem::Read1<int>(g_locPlayer + OFFSET_TEAM);
	Vector localOrigin = mem::Read1<Vector>(g_locPlayer + OFFSET_ORIGIN);

	Manager gm(1); // Ensure this Manager class is efficient and doesn't do deep reads in constructor

	// Buffer for text formatting to avoid std::to_string (heap allocations)
	char textBuffer[32];

	// --- 2. Player ESP ---
	for (MainPlayer& player : gm.Players)
	{
		if (player.ptr == 0 || player.Health <= 0) continue;

		int Team = mem::Read1<int>(player.ptr + OFFSET_TEAM);
		if (Team == LocalTeam && !Settings.Team) continue;

		Vector entFeet = GetEntityBonePosition(player.ptr, 8);
		Vector w2sEntFeet = _WorldToScreen(entFeet, m);
		if (w2sEntFeet.z <= 0.f) continue;

		int YP = 40;

		Vector origin = mem::Read1<Vector>(player.ptr + OFFSET_ORIGIN);

		float dist = localOrigin.DistTo(origin);
		float meters = ToMeters(dist);
		if (meters > Settings.MaxDistance) continue; // Skip everything below for this player

		// Draw Team
		if (Settings.Team) {
			snprintf(textBuffer, sizeof(textBuffer), "#%d", Team);
			overlay->draw_text_green(w2sEntFeet.x, w2sEntFeet.y - YP, textBuffer);
			YP += 20;
		}

		// Draw Name (Assuming GetName returns a string or char*)
		if (Settings.Name) {
			overlay->draw_text_blue(w2sEntFeet.x, w2sEntFeet.y - YP, GetName(player.ptr).c_str());
			YP += 20;
		}

		bool visible = GetVisible(g_locPlayer, player.ptr);

		// Use your specific function signature
		Vector headPos = GetBonePositionByHitbox(player.ptr, (int)HitboxType::Head, origin);
		Vector feetPos = GetBonePositionByHitbox(player.ptr, (int)HitboxType::Rightleg, origin);

		Vector w2sHead = _WorldToScreen(headPos, m);
		Vector w2sFeet = _WorldToScreen(feetPos, m);

		// --- 2D Box ESP ---
		if (Settings.Box)
		{

			if (w2sHead.z > 0.001f && w2sFeet.z > 0.001f)
			{
				float height = abs(w2sFeet.y - w2sHead.y) * 1.1f;
				float width = height / 2.5f;
				float x = w2sHead.x - (width / 2.0f);
				float y = w2sHead.y - (height * 0.1f);

				auto color = visible ? D2D1::ColorF(D2D1::ColorF::Green) : D2D1::ColorF(D2D1::ColorF::Red);

				if (Settings.BoxType == BoxStyle::Full)
				{
					overlay->draw_rect(x, y, width, height, 1.5f, color);
				}
				else if (Settings.BoxType == BoxStyle::Cornered)
				{
					float cornerW = width / 4.0f;
					float cornerH = height / 4.0f;
					float t = 1.5f;

					// Top Left
					overlay->draw_line(x, y, x + cornerW, y, t, color);
					overlay->draw_line(x, y, x, y + cornerH, t, color);
					// Top Right
					overlay->draw_line(x + width, y, x + width - cornerW, y, t, color);
					overlay->draw_line(x + width, y, x + width, y + cornerH, t, color);
					// Bottom Left
					overlay->draw_line(x, y + height, x + cornerW, y + height, t, color);
					overlay->draw_line(x, y + height, x, y + height - cornerH, t, color);
					// Bottom Right
					overlay->draw_line(x + width, y + height, x + width - cornerW, y + height, t, color);
					overlay->draw_line(x + width, y + height, x + width, y + height - cornerH, t, color);
				}
			}
		}

		if (Settings.Health || Settings.Shield) // Assuming you have a toggle for these
		{
			overlay->Draw2DBar(
				Settings.Health,
				Settings.Shield,
				0, // 0 = Side, 1 = Top
				w2sFeet.x, w2sFeet.y,
				w2sHead.x, w2sHead.y,
				player.Health, 100,
				player.ShieldHealth, 125 // Ensure you read player.Shield from memory
			);
		}

		// --- Skeleton ESP ---
		if (Settings.Skelton)
		{
			// Pre-calculate color (consider storing these as constants/members)
			auto skeletonColor = visible ? D2D1::ColorF(D2D1::ColorF::Green) : D2D1::ColorF(D2D1::ColorF::Red);

			// Optimized GetBone: Return a flag + position to avoid zero-checks later
			auto GetBone = [&](HitboxType type, Vector2D& out) -> bool {
				Vector worldPos = GetBonePositionByHitbox(player.ptr, (int)type, origin);
				Vector screen = _WorldToScreen(worldPos, m);
				if (screen.z < 0.001f) return false;
				out = { screen.x, screen.y };
				return true;
				};

			Vector2D head, neck, uChest, lChest, stomach, hip;
			Vector2D lSh, lEl, lHa, rSh, rEl, rHa;
			Vector2D lTh, lKn, lLe, rTh, rKn, rLe;

			// Only proceed if critical points are visible (e.g., neck and hip)
			if (!GetBone(HitboxType::Neck, neck) || !GetBone(HitboxType::Hip, hip)) continue;

			// Fetch others (optional: add null checks if they aren't critical)
			GetBone(HitboxType::Head, head);
			GetBone(HitboxType::UpperChest, uChest);
			GetBone(HitboxType::LowerChest, lChest);
			GetBone(HitboxType::Stomach, stomach);
			GetBone(HitboxType::Leftshoulder, lSh); GetBone(HitboxType::Leftelbow, lEl); GetBone(HitboxType::Lefthand, lHa);
			GetBone(HitboxType::Rightshoulder, rSh); GetBone(HitboxType::RightelbowBone, rEl); GetBone(HitboxType::Righthand, rHa);
			GetBone(HitboxType::LeftThighs, lTh); GetBone(HitboxType::Leftknees, lKn); GetBone(HitboxType::Leftleg, lLe);
			GetBone(HitboxType::RightThighs, rTh); GetBone(HitboxType::Rightknees, rKn); GetBone(HitboxType::Rightleg, rLe);

			auto Draw = [&](const Vector2D& s, const Vector2D& e) {
				if (s.x != 0 && e.x != 0) overlay->draw_line(s.x, s.y, e.x, e.y, 1.0f, skeletonColor);
				};

			// Spine
			Draw(head, neck); Draw(neck, uChest); Draw(uChest, lChest); Draw(lChest, stomach); Draw(stomach, hip);
			// Arms
			Draw(neck, lSh); Draw(lSh, lEl); Draw(lEl, lHa);
			Draw(neck, rSh); Draw(rSh, rEl); Draw(rEl, rHa);
			// Legs
			Draw(hip, lTh); Draw(lTh, lKn); Draw(lKn, lLe);
			Draw(hip, rTh); Draw(rTh, rKn); Draw(rKn, rLe);
		}
	}


	if (Settings.ItemDist || Settings.ItemName) {
		const Vector localPlayerPos = mem::Read1<Vector>(g_locPlayer + OFFSET_ORIGIN);

		for (const Loot& loot : gm.Loots) {
			// Use equal_range to get all items with this ID
			auto range = LootItems::itemLists.equal_range(loot.nameid);

			// Iterate through all items found for this ID
			for (auto it = range.first; it != range.second; ++it) {
				const auto& itemData = it->second;

				Vector w2s = _WorldToScreen(loot.origin, m);
				if (w2s.z <= 0.f) continue;

				// Prepare strings
				std::string nameStr = itemData.itemName;
				std::string distStr = "";
				if (Settings.ItemDist) {
					float meters = ToMeters(localPlayerPos.DistTo(loot.origin));
					distStr = "[" + std::to_string(static_cast<int>(meters)) + "M]";
				}

				// DRAWING LOGIC
				auto drawFunc = [&](auto draw_fn) {
					// Print the name being retrieved
					// If this prints empty space, itemData.itemName is empty!
					draw_fn(w2s.x, w2s.y, itemData.itemName.c_str());

					if (Settings.ItemDist)
						draw_fn(w2s.x, w2s.y + 15, distStr.c_str());
					};

				// Rarity Switch (Same as before)
				switch (itemData.rarity) {
				case LootItems::RARE:      drawFunc([&](auto... args) { overlay->draw_text_blue(args...); }); break;
				case LootItems::EPIC:      drawFunc([&](auto... args) { overlay->draw_text_purple(args...); }); break;
				case LootItems::LEGENDARY: drawFunc([&](auto... args) { overlay->draw_text_yellow(args...); }); break;
				case LootItems::HEIRLOOM:  drawFunc([&](auto... args) { overlay->draw_text_red(args...); }); break;
				default:                   drawFunc([&](auto... args) { overlay->draw_text_white(args...); }); break;
				}
			}
		}
	}
}

// --- Global Theme Definition ---
struct Theme {
	D2D1::ColorF Background;
	D2D1::ColorF Accent;
	D2D1::ColorF Highlight;
};

// Available Themes (8 Total)
Theme ThemeList[] = {
	{ D2D1::ColorF(0.05f, 0.05f, 0.05f, 0.95f), D2D1::ColorF(0.2f, 0.6f, 1.0f, 1.0f), D2D1::ColorF(0.2f, 0.6f, 1.0f, 0.3f) }, // 0: Blue
	{ D2D1::ColorF(0.05f, 0.05f, 0.05f, 0.95f), D2D1::ColorF(0.2f, 0.8f, 0.2f, 1.0f), D2D1::ColorF(0.2f, 0.8f, 0.2f, 0.3f) }, // 1: Green
	{ D2D1::ColorF(0.05f, 0.05f, 0.05f, 0.95f), D2D1::ColorF(0.8f, 0.2f, 0.8f, 1.0f), D2D1::ColorF(0.8f, 0.2f, 0.8f, 0.3f) }, // 2: Purple
	{ D2D1::ColorF(0.05f, 0.05f, 0.05f, 0.95f), D2D1::ColorF(0.9f, 0.2f, 0.2f, 1.0f), D2D1::ColorF(0.9f, 0.2f, 0.2f, 0.3f) }, // 3: Red
	{ D2D1::ColorF(0.05f, 0.05f, 0.05f, 0.95f), D2D1::ColorF(1.0f, 0.9f, 0.0f, 1.0f), D2D1::ColorF(1.0f, 0.9f, 0.0f, 0.3f) }, // 4: Yellow
	{ D2D1::ColorF(0.05f, 0.05f, 0.05f, 0.95f), D2D1::ColorF(0.0f, 1.0f, 1.0f, 1.0f), D2D1::ColorF(0.0f, 1.0f, 1.0f, 0.3f) }, // 5: Cyan
	{ D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.95f),    D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.2f) }, // 6: Contrast
	{ D2D1::ColorF(0.1f, 0.0f, 0.1f, 0.95f),    D2D1::ColorF(1.0f, 0.0f, 0.5f, 1.0f), D2D1::ColorF(1.0f, 0.0f, 0.5f, 0.4f) }  // 7: CYBERPUNK
};

static int CurrentThemeIndex = 0;
const int THEME_COUNT = 8;
Theme CurrentTheme = ThemeList[0];
static int selectedIndex = 0;
const int MAX_MENU_ITEMS = 9;

void DrawMenu(CheatOverlay* overlay)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		Settings.bMenuOpen = !Settings.bMenuOpen;
	}

	if (!Settings.bMenuOpen)
	{
		overlay->draw_text_white(10, 10, skCrypt(u8"PRESS [INSERT] TO OPEN MENU"));
		return;
	}

	// 1. Navigation Handling (Up/Down)
	if (GetAsyncKeyState(VK_UP) & 1)
		selectedIndex = (selectedIndex - 1 + MAX_MENU_ITEMS) % MAX_MENU_ITEMS;
	if (GetAsyncKeyState(VK_DOWN) & 1)
		selectedIndex = (selectedIndex + 1) % MAX_MENU_ITEMS;

	// 2. Action Handling (Left/Right)
	if (selectedIndex == 6) // BoxType Selection
	{
		if ((GetAsyncKeyState(VK_LEFT) & 1) || (GetAsyncKeyState(VK_RIGHT) & 1))
			Settings.BoxType = (Settings.BoxType == BoxStyle::Full) ? BoxStyle::Cornered : BoxStyle::Full;
	}
	else if (selectedIndex == 8) // Theme Selection
	{
		if (GetAsyncKeyState(VK_RIGHT) & 1)
			CurrentThemeIndex = (CurrentThemeIndex + 1) % THEME_COUNT;
		if (GetAsyncKeyState(VK_LEFT) & 1)
			CurrentThemeIndex = (CurrentThemeIndex - 1 + THEME_COUNT) % THEME_COUNT;

		CurrentTheme = ThemeList[CurrentThemeIndex];
	}
	else if (selectedIndex == 7) // Max Distance Adjustment
	{
		float step = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 10.0f : 1.0f;

		if (GetAsyncKeyState(VK_RIGHT) & 1) Settings.MaxDistance += step;
		if (GetAsyncKeyState(VK_LEFT) & 1) Settings.MaxDistance -= step;

		if (Settings.MaxDistance < 10.0f) Settings.MaxDistance = 10.0f;
		if (Settings.MaxDistance > 2000.0f) Settings.MaxDistance = 2000.0f;
	}
	else // Feature Toggle Rows
	{
		if ((GetAsyncKeyState(VK_LEFT) & 1) || (GetAsyncKeyState(VK_RIGHT) & 1))
		{
			bool* settingsArr[] = { &Settings.Health, &Settings.Name, &Settings.Skelton,
									&Settings.Distance, &Settings.ItemDist, &Settings.Box };
			*settingsArr[selectedIndex] = !(*settingsArr[selectedIndex]);
		}
	}

	// Visual Settings
	float menuX = 50.0f, menuY = 100.0f;
	float menuW = 300.0f, menuH = 415.0f;

	overlay->draw_filled_box(menuX, menuY, menuW, menuH, CurrentTheme.Background);
	overlay->draw_outlined_box(menuX, menuY, menuW, menuH, CurrentTheme.Accent, 2.0f);

	float textWidthOffset = 80.0f;
	overlay->draw_text_white(menuX + (menuW / 2) - textWidthOffset, menuY + 15, skCrypt(u8"BALLTIMOR | ESP"));
	overlay->draw_filled_box(menuX + 10, menuY + 40, menuW - 20, 2.0f, CurrentTheme.Accent);

	char textBuffer[32];

	// Items
	for (int i = 0; i < MAX_MENU_ITEMS; i++) {
		float y = menuY + 60 + (i * 35);

		if (selectedIndex == i) {
			overlay->draw_filled_box(menuX + 10, y - 5, menuW - 20, 25.0f, CurrentTheme.Highlight);
			overlay->draw_text_white(menuX + 15, y, skCrypt(u8">"));
		}

		if (i < 6) { // Toggles
			const char* labels[] = { "HEALTH ESP", "NAME ESP", "SKELETON", "DISTANCE", "ITEMS", "BOX ESP" };
			bool* states[] = { &Settings.Health, &Settings.Name, &Settings.Skelton, &Settings.Distance, &Settings.ItemDist, &Settings.Box };

			// Applying skCrypt here using a wrapper if your library expects raw strings or requires manual casting
			overlay->draw_text_white(menuX + 35, y, labels[i]); // Ensure labels are processed if not already
			if (*states[i]) overlay->draw_text_green(menuX + 210, y, skCrypt(u8"[ON]"));
			else overlay->draw_text_red(menuX + 210, y, skCrypt(u8"[OFF]"));
		}
		else if (i == 6) { // BoxType
			overlay->draw_text_white(menuX + 35, y, skCrypt(u8"BOX TYPE"));
			overlay->draw_text_white(menuX + 180, y, (Settings.BoxType == BoxStyle::Full) ? skCrypt(u8"[FULL]") : skCrypt(u8"[CORNER]"));
		}
		else if (i == 7) { // Max Distance
			overlay->draw_text_white(menuX + 35, y, skCrypt(u8"ESP MAX DISTANCE"));
			snprintf(textBuffer, sizeof(textBuffer), skCrypt(u8"%.0f M"), Settings.MaxDistance);
			overlay->draw_text_white(menuX + 190, y, textBuffer);
		}
		else if (i == 8) { // Theme
			overlay->draw_text_white(menuX + 35, y, skCrypt(u8"THEME SELECT"));
			const char* themeNames[] = { skCrypt(u8"[BLUE]"), skCrypt(u8"[GREEN]"), skCrypt(u8"[PURPLE]"), skCrypt(u8"[RED]"), skCrypt(u8"[YELLOW]"), skCrypt(u8"[CYAN]"), skCrypt(u8"[CONTRAST]"), skCrypt(u8"[CYBER]") };
			overlay->draw_text_white(menuX + 160, y, themeNames[CurrentThemeIndex]);
		}
	}

	overlay->draw_filled_box(menuX + 10, menuY + 385, menuW - 20, 1.0f, CurrentTheme.Accent);
	overlay->draw_text_white(menuX + 20, menuY + 390, skCrypt(u8"UP/DOWN : NAV | LEFT/RIGHT : TOGGLE/ADJ"));
}

static void _init(CheatOverlay* overlay)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// --- Header ---
	SetColor(hConsole, CYAN);
	std::cout << skCrypt("==========================================") << std::endl;
	std::cout << skCrypt("       BALLTIMOR ESP INITIALIZATION       ") << std::endl;
	std::cout << skCrypt("==========================================") << std::endl;

	// 1. Initialize Driver
	if (!mem::initdriver()) {
		SetColor(hConsole, RED);
		std::cerr << skCrypt("[!] Failed to initialize driver.") << std::endl;
		Sleep(3000);
		return;
	}
	SetColor(hConsole, GREEN);
	std::cout << skCrypt("[+] Driver Initialized successfully.") << std::endl;

	// 2. Setup Process
	get_process_id = mem::GetPID("r5apex_dx12.exe");
	if (!get_process_id) {
		SetColor(hConsole, RED);
		std::cerr << skCrypt("[-] Process 'r5apex_dx12.exe' not found!") << std::endl;
		Sleep(3000);
		return;
	}

	get_cr3 = mem::get_cr3();
	SetColor(hConsole, GREEN);
	std::cout << skCrypt("[+] Process Found (PID: ") << get_process_id << skCrypt(")") << std::endl;

	// 3. Find Base Address
	GameBase = mem::find_base_address();
	if (!GameBase)
	{
		SetColor(hConsole, RED);
		std::cerr << skCrypt("[!] Critical Error: Failed to get module base address!") << std::endl;
		SetColor(hConsole, WHITE);
		Sleep(5000);
		exit(1);
	}

	SetColor(hConsole, YELLOW);
	std::cout << skCrypt("[+] Base Address: 0x") << std::hex << GameBase << std::dec << std::endl;
	nameList = GameBase + OFFSET_NAMELIST;

	// 4. Background Setup
	std::thread(setupLoot).detach();
	std::thread(setupPlayers).detach();

	// 5. Initialize Window & D2D
	if (!overlay->window_init()) {
		SetColor(hConsole, RED);
		std::cerr << skCrypt("[-] Failed to initialize overlay window.") << std::endl;
		Sleep(2000);
		return;
	}

	if (!overlay->init_d2d()) {
		SetColor(hConsole, RED);
		std::cerr << skCrypt("[-] Direct2D failed to initialize.") << std::endl;
		Sleep(2000);
		return;
	}

	SetColor(hConsole, WHITE);

	// 6. Main Loop
	while (true)
	{
		MSG msg = { 0 };
		if (PeekMessage(&msg, overlay->retrieve_window(), 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		overlay->begin_scene();
		overlay->clear_scene();

		DrawMenu(overlay);
		DrawESP(overlay);

		overlay->end_scene();

		Sleep(15);
	}

	overlay->d2d_shutdown();
}

int main() {
	CheatOverlay overlay_obj;
	_init(&overlay_obj);
}