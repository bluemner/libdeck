/**
 * SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (c) 2026 Brandon Bluemner <dev@brandonbluemner.com>
 */
#include "deck.h"
// Sets the bit at 'button_idx' to 1
#define SET_BUTTON(map, button_idx) ((map)->buttons |= (1u << (button_idx)))

// Sets the bit at 'button_idx' to 0
#define CLR_BUTTON(map, button_idx) ((map)->buttons &= ~(1u << (button_idx)))

// Toggle a bit
#define TGL_BUTTON(map, button_idx) ((map)->buttons ^= (1u << (button_idx)))

// Returns 1 if the bit is set, 0 otherwise
#define GET_BUTTON(map, button_idx) (((map)->buttons >> (button_idx)) & 1u)

typedef struct input_map
{
    unsigned int buttons : SDL_CONTROLLER_BUTTON_MAX;
    unsigned int left_pad : 1;
    unsigned int right_pad : 1;
    unsigned int left_stick : 1;
    unsigned int right_stick : 1;
} input_map;


void DrawCircleGeometry(SDL_Renderer* renderer, float centerX, float centerY, float radius, int segments) {
    // We need (segments + 1) vertices: 1 for center, and 'segments' for the edge
    int num_vertices = segments + 1;
    SDL_Vertex vertices[num_vertices];

    // Center point
    vertices[0].position = (SDL_FPoint) { centerX, centerY };
    vertices[0].color = (SDL_Color) { 255, 255, 255, 255 }; // White
    vertices[0].tex_coord =  (SDL_FPoint) { 0, 0 };

    // Edge points
    for (int i = 0; i < segments; i++) {
        float angle = i * 2.0f * M_PI / (segments - 1);
        vertices[i + 1].position =  (SDL_FPoint) { centerX + radius * cosf(angle), centerY + radius * sinf(angle) };
        vertices[i + 1].color = (SDL_Color) { 255, 255, 255, 255 };
        vertices[i + 1].tex_coord = (SDL_FPoint) { 0, 0 };
    }

    // Create indices to connect the center to the edges (forming triangles)
    int num_indices = segments * 3;
    int indices[num_indices];
    for (int i = 0; i < segments; i++) {
        indices[i * 3] = 0;             // Always start at center
        indices[i * 3 + 1] = i + 1;     // Current edge point
        indices[i * 3 + 2] = (i + 1) % segments + 1; // Next edge point
    }

    SDL_RenderGeometry(renderer, NULL, vertices, num_vertices, indices, num_indices);
}

void DrawAAFilledCircle(SDL_Renderer* renderer, SDL_Texture* circleTexture, float x, float y, float radius, SDL_Color color) {
    // Define the 4 corners of the square (2 triangles)
    SDL_Vertex vertices[4];
    
    // Set color and texture coordinates for all 4 vertices
    for(int i = 0; i < 4; i++) {
        vertices[i].color = (SDL_Color){color.r, color.g, color.b, color.a};
    }

    // Top-Left
    vertices[0].position = (SDL_FPoint){ x - radius, y - radius };
    vertices[0].tex_coord = (SDL_FPoint){ 0.0f, 0.0f };
    // Top-Right
    vertices[1].position = (SDL_FPoint){ x + radius, y - radius };
    vertices[1].tex_coord = (SDL_FPoint){ 1.0f, 0.0f };
    // Bottom-Left
    vertices[2].position = (SDL_FPoint){ x - radius, y + radius };
    vertices[2].tex_coord = (SDL_FPoint){ 0.0f, 1.0f };
    // Bottom-Right
    vertices[3].position = (SDL_FPoint){ x + radius, y + radius };
    vertices[3].tex_coord = (SDL_FPoint){ 1.0f, 1.0f };

    // Indices for two triangles making a square (0-1-2 and 1-2-3)
    int indices[] = { 0, 1, 2, 1, 2, 3 };

    // Draw the textured geometry
    SDL_RenderGeometry(renderer, circleTexture, vertices, 4, indices, 6);
}

SDL_Texture* CreateSmoothCircleTexture(SDL_Renderer* renderer, int size) {
    // Create a 32-bit RGBA surface
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
    
    float center = (size - 1) / 2.0f;
    float radius = size / 2.0f;
    Uint32* pixels = (Uint32*)surface->pixels;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = x - center;
            float dy = y - center;
            float distance = sqrtf(dx * dx + dy * dy);

            Uint8 alpha = 0;
            if (distance < radius - 1.0f) {
                alpha = 255; // Solid interior
            } else if (distance < radius) {
                // Anti-aliasing: smooth transition for the edge pixel
                alpha = (Uint8)((radius - distance) * 255.0f);
            }

            // SDL_PIXELFORMAT_RGBA32 is typically 0xRRGGBBAA or 0xAABBGGRR depending on endianness
            // Using SDL_MapRGBA is safer across different platforms
            pixels[y * size + x] = SDL_MapRGBA(surface->format, 255, 255, 255, alpha);
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND); // Enable transparency
    SDL_FreeSurface(surface);
    return texture;
}

void DrawCircle(SDL_Renderer* renderer, int x, int y, int size, float radius, SDL_Color color ){
    SDL_Texture* circleTex = CreateSmoothCircleTexture(renderer, size);
    SDL_SetTextureColorMod(circleTex, color.r, color.g, color.b); 
    SDL_SetTextureAlphaMod(circleTex, color.a);
    DrawAAFilledCircle(renderer, circleTex, x, y, radius, color);
}

void DrawRoundedBox(SDL_Renderer *renderer, SDL_Rect rect, int rad, SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Main Body Cross
    SDL_Rect vRect = {rect.x + rad, rect.y, rect.w - 2 * rad, rect.h};
    SDL_Rect hRect = {rect.x, rect.y + rad, rect.w, rect.h - 2 * rad};
    SDL_RenderFillRect(renderer, &vRect);
    SDL_RenderFillRect(renderer, &hRect);

    // Filling corners with horizontal lines
    int cx = 0, cy = rad, d = 3 - 2 * rad;
    while (cx <= cy)
    {
        SDL_RenderDrawLine(renderer, rect.x + rad - cy, rect.y + rad - cx, rect.x + rect.w - rad + cy - 1, rect.y + rad - cx);
        SDL_RenderDrawLine(renderer, rect.x + rad - cx, rect.y + rad - cy, rect.x + rect.w - rad + cx - 1, rect.y + rad - cy);
        SDL_RenderDrawLine(renderer, rect.x + rad - cy, rect.y + rect.h - rad + cx - 1, rect.x + rect.w - rad + cy - 1, rect.y + rect.h - rad + cx - 1);
        SDL_RenderDrawLine(renderer, rect.x + rad - cx, rect.y + rect.h - rad + cy - 1, rect.x + rect.w - rad + cx - 1, rect.y + rect.h - rad + cy - 1);
        if (d < 0)
            d = d + 4 * cx + 6;
        else
        {
            d = d + 4 * (cx - cy) + 10;
            cy--;
        }
        cx++;
    }
}

void DrawRoundedBoxWireframe(SDL_Renderer *renderer, SDL_Rect rect, int rad, SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Main Body Cross
    //SDL_Rect vRect = {rect.x + rad, rect.y, rect.w - 2 * rad, rect.h};
    SDL_Rect hRect = {rect.x, rect.y + rad, rect.w, rect.h - 2 * rad};
    //SDL_RenderDrawRect(renderer, &vRect);
    SDL_RenderDrawRect(renderer, &hRect);

    // Filling corners with horizontal lines
    int cx = 0, cy = rad, d = 3 - 2 * rad;
    while (cx <= cy)
    {
        SDL_RenderDrawLine(renderer, rect.x + rad - cy, rect.y + rad - cx, rect.x + rect.w - rad + cy - 1, rect.y + rad - cx);
        SDL_RenderDrawLine(renderer, rect.x + rad - cx, rect.y + rad - cy, rect.x + rect.w - rad + cx - 1, rect.y + rad - cy);
        SDL_RenderDrawLine(renderer, rect.x + rad - cy, rect.y + rect.h - rad + cx - 1, rect.x + rect.w - rad + cy - 1, rect.y + rect.h - rad + cx - 1);
        SDL_RenderDrawLine(renderer, rect.x + rad - cx, rect.y + rect.h - rad + cy - 1, rect.x + rect.w - rad + cx - 1, rect.y + rect.h - rad + cy - 1);
        if (d < 0)
            d = d + 4 * cx + 6;
        else
        {
            d = d + 4 * (cx - cy) + 10;
            cy--;
        }
        cx++;
    }
}

// --- Additional Helper for the D-Pad ---
void DrawDPad(SDL_Renderer *renderer, int x, int y, int size, int scale, SDL_Color color, input_map *input)
{
    int thickness = size / 3;
    int half = size / 2;
    SDL_Rect left = {x, y + thickness, half, thickness};
    SDL_Rect right = {x + half, y + thickness, half, thickness};
    SDL_Rect top = {x + thickness, y, thickness, half};
    SDL_Rect down = {x + thickness, y + half, thickness, half};

    SDL_Color btnColSelected = {11, 64, 222, 255};
    DrawRoundedBox(renderer, top, 4 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_DPAD_UP) ? btnColSelected : color);
    DrawRoundedBox(renderer, down, 4 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ? btnColSelected : color);
    DrawRoundedBox(renderer, left, 4 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ? btnColSelected : color);
    DrawRoundedBox(renderer, right, 4 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? btnColSelected : color);

    // Middle fix
    SDL_Rect box = {x + thickness, y + thickness, thickness, thickness};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &box);
}

void drawTrackPad(SDL_Renderer *renderer, int x, int y, int scale, int trackPadSize,  bool selected){
    SDL_Rect rect = {x, y, trackPadSize * scale, trackPadSize * scale};
    SDL_Color trackPadColor = {35, 35, 35, 255};
    SDL_Color trackPadColorSelected = {11, 64, 222, 255};
    DrawRoundedBox(renderer, rect, 8 * scale, selected ? trackPadColorSelected : trackPadColor);
}

void drawButtons(SDL_Renderer *renderer, int x, int y, int scale, input_map *input){    
    int btnSize = 24 * scale; // 18
    int anchorX = x ;// + 560 *scale
    int anchorY = y ; // + 25 *scale // Adjusted Y for the button cluster
    int offset = 24 * scale;      // 22

    SDL_Color btnCol = {60, 60, 60, 255};
    SDL_Color btnColSelected = {11, 64, 222, 255};
    // Y (Top), A (Bottom), X (Left), B (Right)
    SDL_Rect btnY = {anchorX, anchorY - offset, btnSize, btnSize};
    SDL_Rect btnA = {anchorX, anchorY + offset, btnSize, btnSize};
    SDL_Rect btnX = {anchorX - offset, anchorY, btnSize, btnSize};
    SDL_Rect btnB = {anchorX + offset, anchorY, btnSize, btnSize};

    DrawRoundedBox(renderer, btnY, 9 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_Y) ? btnColSelected : btnCol);
    DrawRoundedBox(renderer, btnA, 9 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_A) ? btnColSelected : btnCol);
    DrawRoundedBox(renderer, btnX, 9 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_X) ? btnColSelected : btnCol);
    DrawRoundedBox(renderer, btnB, 9 * scale, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_B) ? btnColSelected : btnCol);

}

void drawJoyStick(SDL_Renderer *renderer, int x, int y, int scale, bool pressed, bool active ){
    int button_size = 50;

    SDL_Rect lStick = {x , y, 50 * scale, 50 * scale};
    SDL_Color joyColor = {20, 20, 20, 255};
    SDL_Color joyColorStickSelected = {11, 64, 222, 255};
    SDL_Color joyColorShoulder = {100, 00, 00, 255};

    SDL_Color joyColorLeft = joyColor;
    if (pressed)
    {
        joyColorLeft = joyColorStickSelected;
    }
    else if (active)
    {
        joyColorLeft = joyColorShoulder;
    }

    DrawRoundedBox(renderer, lStick, button_size / 2 * scale, joyColorLeft);

}

// --- Enhanced Steam Deck Drawing Logic ---
void drawSteamDeck(SDL_Renderer *renderer, int x, int y, float scale, input_map *input)
{

    //DrawCircleGeometry(renderer, 400.0f, 300.0f, 100.0f, 60);

    x = 80 * scale, 
    y = 180 * scale;
    
    SDL_Point body_size = { .x =1120, .y = 420 };

    // Main Body
    SDL_Rect body = {x, y, body_size.x * scale, body_size.y * scale};
    SDL_Color bodyColor = {45, 45, 45, 255};
    DrawRoundedBox(renderer, body, 40 * scale, bodyColor);


    // Screen (Center)
    SDL_Rect screen = {x + 200 * scale, y + 10 * scale, (body_size.x-400) * scale, (body_size.y-20) * scale};
    SDL_Color screenColor = {10, 10, 10, 255};
    DrawRoundedBox(renderer, screen, 5 * scale, screenColor);


    SDL_Point screen_size = {
        .x = screen.x + screen.w,
        .y = screen.y + screen.h
    };

    // Track Pads
    int trackPadSize = 110;
    // Left
    drawTrackPad(renderer, (screen.x - 20 *scale - (trackPadSize * scale) ), y + 130 * scale, scale , trackPadSize, input->left_pad);
    // Right
    drawTrackPad(renderer, (screen_size.x + 20 *scale), y + 130 * scale, scale , trackPadSize, input->right_pad);
   
    // D-Pad (Left Side, above trackpad)
    SDL_Color dPadColor = {30, 30, 30, 255};
    DrawDPad(renderer, x + 10 * scale, y + 10 * scale, 60 * scale, scale, dPadColor, input);

    // ABXY Buttons (Right Side, above trackpad)   
    drawButtons(renderer, body.x + body.w - 60 *scale, y + 35 *scale, scale, input);

    // Joysticks (Small simple circles for placement)
    drawJoyStick(renderer, (screen.x - 20 *scale - (50 * scale) ), y + 25 * scale, scale,  GET_BUTTON(input, SDL_CONTROLLER_BUTTON_LEFTSTICK), input->left_stick);
    drawJoyStick(renderer, (screen_size.x + 20 *scale ), y + 25 * scale, scale,  GET_BUTTON(input, SDL_CONTROLLER_BUTTON_RIGHTSTICK), input->right_stick);

    int smallBtnSizeX = 30 * scale;
    int smallBtnSizeY = 10 * scale;
    SDL_Color smallBtnCol = {60, 60, 60, 255};
    SDL_Color smallBtnColSelected = {11, 64, 222, 255};

    int smallButtonShift = 75 * scale;
    int smallButtonHeight = 10 * scale; 
    // Select/View (Left side of screen)
    SDL_Rect viewBtn = {(screen.x- (smallBtnSizeX) - (smallButtonShift)), y + smallButtonHeight, smallBtnSizeX, smallBtnSizeY};
    // Start/Menu (Right side of screen)
    SDL_Rect menuBtn = {(screen_size.x + smallButtonShift), y + smallButtonHeight, smallBtnSizeX, smallBtnSizeY};
    int rad = 5 * scale;
    DrawRoundedBox(renderer, viewBtn, rad, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_BACK) ? smallBtnColSelected : smallBtnCol);
    DrawRoundedBox(renderer, menuBtn, rad, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_START) ? smallBtnColSelected : smallBtnCol);

    // 7. Steam & Options Buttons (Bottom corners of screen)
    SDL_Rect steamBtn = {screen.x- (smallBtnSizeX * 2) - (20*scale), y + 270 * scale, smallBtnSizeX * 2, smallBtnSizeY * 2};
    SDL_Rect optionsBtn = {(screen_size.x + 20 *scale), y + 270 * scale, smallBtnSizeX * 2, smallBtnSizeY * 2};
    rad = 6 * scale;
    DrawRoundedBox(renderer, steamBtn, rad, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_MISC1) ? smallBtnColSelected : smallBtnCol);
    DrawRoundedBox(renderer, optionsBtn, rad, GET_BUTTON(input, SDL_CONTROLLER_BUTTON_GUIDE) ? smallBtnColSelected : smallBtnCol);
}

void DrawGrid(SDL_Renderer *renderer, int windowWidth, int windowHeight, int scale)
{
    // int rows = 800, cols = 600;

    SDL_RenderSetLogicalSize(renderer, 1280, 800);
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 50);

    int cellSize = 10;

    int width = windowWidth * scale;
    int height = windowHeight * scale;
    // Draw vertical lines
    for (int x = 0; x <= width; x += cellSize)
    {
        SDL_RenderDrawLine(renderer, x, 0, x, width);
    }

    // Draw horizontal lines
    for (int y = 0; y <= height; y += cellSize)
    {
        SDL_RenderDrawLine(renderer, 0, y, height, y);
    }
}

typedef struct controllers_list
{
    int32_t id;
    SDL_GameController *controller;
    struct controllers_list *next;
    struct controllers_list *previous;
} controllers_list;

void controllers_list_add(controllers_list *root, int32_t id, SDL_GameController *controller)
{
    controllers_list *node = malloc(sizeof(controllers_list));
    root->previous->next = node;
    node->next = root;
    root->previous = node;
    node->controller = controller;
    node->id = id;
}

void controllers_list_remove(controllers_list *root, Sint32 id)
{
    controllers_list *node = root;
    while (node->id != id)
    {
        node = root->next;
        if (node == root)
        {
            return;
        }
    }
    node->previous->next = node->next;
    node->next->previous = node->previous;
    node->next = nullptr;
    node->previous = nullptr;
    if (node->controller != nullptr)
    {
        SDL_GameControllerClose(node->controller);
    }
    free(node);
}

void controllers_list_init(controllers_list *root)
{
    root->id = -1;
    root->controller = nullptr;
    root->next = root;
    root->previous = root;
}

void controllers_list_delete(controllers_list *root)
{
    controllers_list *node = root->next;
    while (node != root)
    {
        controllers_list *current = node;
        debug_printf("Root [%d] Node[%d]\n", root->id, current->id);

        if (node->controller != nullptr)
        {
            SDL_GameControllerClose(current->controller);
        }
        node = node->next;
        free(current);
    }
}

void quit()
{
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
}

void drawReady(SDL_Renderer *renderer, int scale, bool ready)
{
    SDL_Rect body = {0 + 10 * scale, 0 + 10 * scale, 35 * scale, 35 * scale};
    SDL_Color bodyNotReady = {45, 45, 45, 255};
    SDL_Color bodyReady = {0, 235, 47, 255};
    DrawRoundedBox(renderer, body, 40 * scale, (ready) ? bodyReady : bodyNotReady);
}

// Unicode hex for Material Symbols
// e835
const char *ICON_CHECKED = "\xEE\xA0\xB4";   // 'check_box'
const char *ICON_UNCHECKED = "\xEE\xA0\xB5"; // 'check_box_outline_blank'
const char *ICON_SETTINGS = "\xEE\xA2\xB8";  // Settings
const char *ICON_VOLUME_UP = "\xEE\x81\x90";
const char *ICON_VOLUME_DOWN = (const char *)u8"\ue04d"; //"\xEE\x81\x90"; // e04d
const char *ICON_VOLUME_MUTE = (const char *)u8"\ue04f";

const char *ICON_PLAY_PAUSE = (const char *)u8"\uf137";

const char *ICON_BRIGHTNESS_7 = (const char *)u8"\ue3ac";
const char *ICON_BRIGHTNESS_0 = (const char *)u8"\uf7e8";

const char *ICON_FAST_FORWARD = (const char *)u8"\ue01f";
const char *ICON_FAST_REWIND = (const char *)u8"\ue020";

const char *ICON_BLUETOOTH_OFF = (const char *)u8"\uE1A7";
const char *ICON_BLUETOOTH_ON = (const char *)u8"\ue1a8";

const char *ICON_WIFI_ON = (const char *)u8"\ue63e";
const char *ICON_WIFI_OFF = (const char *)u8"\ue648";

const char *ICON_USB_ON = (const char *)u8"\ue1e0";
const char *ICON_USB_OFF = (const char *)u8"\ue4fa";


enum button_font_size{
    BUTTON_FONT_SIZE_X_SMALL=16,
    BUTTON_FONT_SIZE_SMALL=32,
    BUTTON_FONT_SIZE_MEDIUM=48,
    BUTTON_FONT_SIZE_LARGE=64,
    BUTTON_FONT_SIZE_X_LARGE=128
} button_font_size;

typedef struct Fonts
{
    TTF_Font *text;
    TTF_Font *icon;

    TTF_Font *icon32;
    TTF_Font *icon48;
    TTF_Font *icon64;
    TTF_Font *icon128;

} Fonts_t;

void fonts_open(Fonts_t *fonts, const char *dir, int font_size)
{

    char path[4096];
    snprintf(path, sizeof(path), "%s/%s", dir, "Roboto-Regular.ttf");
    debug_printf("Loading %s\n", path);
    fonts->text = TTF_OpenFont(path, font_size);
    if (fonts->text == nullptr)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    }
    snprintf(path, sizeof(path), "%s/%s", dir, "MaterialSymbolsOutlined.ttf");
    debug_printf("Loading %s\n", path);

    fonts->icon32 = TTF_OpenFont(path, 32);
    if (fonts->icon32 == nullptr)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    }
    fonts->icon = fonts->icon32;

    fonts->icon48 = TTF_OpenFont(path, 48);
    if (fonts->icon48 == nullptr)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    }

    fonts->icon64 = TTF_OpenFont(path, 64);
    if (fonts->icon64 == nullptr)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    }
    
    fonts->icon128 = TTF_OpenFont(path, 128);
    if (fonts->icon128 == nullptr)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    }
}

void fonts_close(Fonts_t *fonts)
{
    TTF_CloseFont(fonts->text);
    TTF_CloseFont(fonts->icon32);
    TTF_CloseFont(fonts->icon48);
    TTF_CloseFont(fonts->icon64);
    TTF_CloseFont(fonts->icon128);
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color, SDL_Rect *rect)
{

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (surf == nullptr)
    {
        // fprintf(stderr, "TTF_RenderUTF8_Blended Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = {rect->x, rect->y, surf->w, surf->h};

    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

typedef struct SettingMenuOption
{
    char *label;
    bool enabled;
    SDL_Rect rect; // Clickable area
} SettingMenuOption_t;

SDL_Color SDL_COLOR_WHITE = {255, 255, 255, 255};
SDL_Color SDL_COLOR_BLUE = {11, 64, 222, 255};
SDL_Color SDL_COLOR_GRAY = {20, 20, 20, 255};
SDL_Color SDL_COLOR_LIGHT_GRAY = {30, 30, 30, 255};

void draw_settings(SDL_Renderer *renderer, SettingMenuOption_t settings[], size_t settings_size, Fonts_t *fonts)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < settings_size; i++)
    {
        debug_printf("Settings %d\n", i);
        SettingMenuOption_t opt = settings[i];
        // 1. Render Icon
        const char *icon = opt.enabled ? ICON_CHECKED : ICON_UNCHECKED;
        renderText(renderer, fonts->icon32, icon, SDL_COLOR_WHITE, &opt.rect);
        // 2. Render Label (Offset to the right of the icon)
        SDL_Rect rect = {opt.rect.x + 50, opt.rect.y + 5, opt.rect.w, opt.rect.h};
        renderText(renderer, fonts->text, opt.label, SDL_COLOR_WHITE, &rect);
    }
}

typedef struct TopButton
{
    bool enabled;
    bool toggle;
    bool hover;
    int size;
    
    const char *icon;
    const char *icon_enabled;
    SDL_Rect rect; // Clickable area
    SDL_Point offset;
    
    SDL_Keycode key;
    SDL_Scancode scan;
    SDL_Color color;
    

} TopButton_t;

void resize_buttonsleft(TopButton_t *buttons, uint8_t count, uint8_t button_size)
{
    TopButton_t *btn;
    for (int i = 0; i < count; i++)
    {
        btn = &buttons[i];
        btn->rect.w = button_size;
        btn->rect.h = button_size;
        btn->size = button_size;
        btn->offset.x = +( (button_size + 18 ) * ( i ));
    }
}

void resize_buttons(TopButton_t *buttons, uint8_t count, uint8_t button_size)
{
    TopButton_t *btn;
    for (int i = 0; i < count; i++)
    {
        btn = &buttons[i];
        btn->rect.w = button_size;
        btn->rect.h = button_size;
        btn->size = button_size;
        btn->offset.x = -( (button_size + 18 ) * ( i+1 ));
    }
}



void render(deck_input_t *deck_input)
{
    SDL_Init(SDL_INIT_EVERYTHING); // SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS
    TTF_Init();
    // 1280 x 800
    int winW = 1280, winH = 800, scale = 16;

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // 4x AA

    SDL_Window *win = SDL_CreateWindow("Bluetooth Controller", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED_MASK, winW, winH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL) ;
    SDL_Renderer *render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    // Create high-res canvas for Super-Sampling
    SDL_Texture *canvas = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, winW * scale, winH * scale);
    SDL_SetTextureScaleMode(canvas, SDL_ScaleModeLinear); // Essential for AA

    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS; // 33ms per frame
    # ifdef __FRAME_DELAY__
        uint32_t frameStart;
        int frameTime;
    # endif
    Sint16 leftStick = 0;
    Sint16 rightStick = 0;

    input_map input = {
        .buttons = false,
        .left_pad = false,
        .right_pad = false
    };

    controllers_list root;
    controllers_list_init(&root);

    Fonts_t fonts;
    fonts_open(&fonts, deck_input->dir, 24);

    SettingMenuOption_t settings[] = {
        {.label = "Full Screen", .enabled = false, .rect = {100, 100, 300, 40}},
        {.label = "Grid", .enabled = false, .rect = {100, 150, 300, 40}},
        {.label = "Show Deck", .enabled = true, .rect = {100, 200, 300, 40}},
        {.label = "Big Buttons", .enabled = true, .rect = {100, 250, 300, 40}},
        {.label = "Button Box", .enabled = true, .rect = {100, 300, 300, 40}},
    };

    SettingMenuOption_t *FullScreen = &settings[0];
    SettingMenuOption_t *GridMenuItem = &settings[1];
    SettingMenuOption_t *ShowDeck = &settings[2];
    SettingMenuOption_t *BigButton = &settings[3];
    SettingMenuOption_t *ButtonBox = &settings[4];

    size_t settings_count = sizeof(settings) / sizeof(SettingMenuOption_t);

    uint8_t BUTTON_SIZE = (u_int8_t) (BigButton->enabled) ? BUTTON_FONT_SIZE_MEDIUM : BUTTON_FONT_SIZE_SMALL;
    fonts.icon = (BigButton->enabled) ? fonts.icon48 : fonts.icon32;
    TopButton_t left_buttons[] = {
        {
        .icon = ICON_BLUETOOTH_OFF,
        .icon_enabled = ICON_BLUETOOTH_ON,
        .enabled = deck_input->running,
        .rect = {10, 10, BUTTON_SIZE, BUTTON_SIZE},
        .key = SDLK_BLUETOOTH,
        .scan= SDL_SCANCODE_BLUETOOTH,
        .hover = false,
        .offset = {10, 10},
        .toggle = true,
        },
        {
        .icon =ICON_WIFI_OFF,
        .icon_enabled =ICON_WIFI_ON,
        .enabled = deck_input->wifi,
        .rect = {10, 10, BUTTON_SIZE, BUTTON_SIZE},
        .key = SDLK_WIFI,
        .scan= SDL_SCANCODE_WIFI,
        .hover = false,
        .offset = {10, 10},
        .toggle = false,
        },
        {
        .icon =ICON_USB_OFF,
        .icon_enabled =ICON_USB_ON,
        .enabled = deck_input->usb,
        .rect = {10, 10, BUTTON_SIZE, BUTTON_SIZE},
        .key = SDLK_USB,
        .scan= SDL_SCANCODE_USB,
        .hover = false,
        .offset = {10, 10},
        .toggle = false,
        },

    };

    size_t const left_buttons_size = sizeof(left_buttons) / sizeof(TopButton_t);
    resize_buttonsleft(left_buttons, left_buttons_size, BUTTON_SIZE);
    
    
    SDL_Rect BUTTON_RECT = {winW, 10, BUTTON_SIZE, BUTTON_SIZE};
    TopButton_t right_buttons[] = {
        {.icon = ICON_SETTINGS, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_APP1, .scan = SDL_SCANCODE_APP1, .toggle = true, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
        {.icon = ICON_VOLUME_UP, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_VOLUMEUP, .scan = SDL_SCANCODE_VOLUMEUP, .toggle = false, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
        {.icon = ICON_VOLUME_DOWN, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_VOLUMEDOWN, .scan = SDL_SCANCODE_VOLUMEDOWN, .toggle = false, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
        {.icon = ICON_VOLUME_MUTE, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_MUTE, .scan = SDL_SCANCODE_MUTE, .toggle = false, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
        {.icon = ICON_FAST_FORWARD, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_AUDIONEXT, .scan = SDL_SCANCODE_AUDIONEXT, .toggle = false, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
        {.icon = ICON_PLAY_PAUSE, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_AUDIOPLAY, .scan = SDL_SCANCODE_AUDIOPLAY, .toggle = false, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
        {.icon = ICON_FAST_REWIND, .rect = BUTTON_RECT, .offset = {0, 0}, .key = SDLK_AUDIOPREV, .scan = SDL_SCANCODE_AUDIOPREV, .toggle = false, .color = SDL_COLOR_WHITE, .hover = false, .size = BUTTON_SIZE},
    };
    size_t const right_buttons_size = sizeof(right_buttons) / sizeof(TopButton_t);
    resize_buttons(right_buttons, right_buttons_size, BUTTON_SIZE);
    TopButton_t *btn;


    debug_printf("Number of controllers %d\n", SDL_NumJoysticks());
    // Open first available controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            controllers_list_add(&root, i, SDL_GameControllerOpen(i));
        }
    }
    deck_input->running = true;

    SDL_Event e, f;
    SDL_Point p, mouse_point;
    uint32_t next_render_time = SDL_GetTicks() + 33;
    bool mouse_mode = false;
    // SDL_SetRelativeMouseMode(mouse_mode);

    //SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    // // If using a high-poll mouse, try disabling high-res desktop events
    //SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0"); 
    while (deck_input->running)
    {
        # ifdef __FRAME_DELAY__
        frameStart = SDL_GetTicks(); // Record start time [2]
        # endif
  
        while (SDL_PollEvent(&e))
        {

            if (e.type == SDL_QUIT)
            {
                deck_input->running = false;
            }

            if (deck_input->events != nullptr)
                deck_input->events(&e, (ptr)deck_input);

            switch (e.type)
            {
            case SDL_CONTROLLERBUTTONDOWN:
                SET_BUTTON(&input, e.cbutton.button);
                // buttons[e.cbutton.button] = true;
                break;
            case SDL_CONTROLLERBUTTONUP:
                // buttons[e.cbutton.button] = false;
                CLR_BUTTON(&input, e.cbutton.button);
                break;

            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                case SDLK_w:
                    SET_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_UP);
                    break;
                case SDLK_a:
                    SET_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                    break;
                case SDLK_s:
                    SET_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                    break;
                case SDLK_d:
                    SET_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                    break;
                case SDLK_g:
                    GridMenuItem->enabled = !GridMenuItem->enabled;
                    break;
                case SDLK_m:
                    mouse_mode = !mouse_mode;
                    SDL_SetRelativeMouseMode(mouse_mode);
                    break;
                case SDLK_ESCAPE:
                    if (deck_input->settings)
                    {
                        deck_input->settings = !deck_input->settings;
                    }
                    else
                    {
                        quit();
                    }
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
                case SDLK_w:
                    CLR_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_UP);
                    break;
                case SDLK_a:
                    CLR_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                    break;
                case SDLK_s:
                    CLR_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                    break;
                case SDLK_d:
                    CLR_BUTTON(&input, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                    break;

                    break;
                default:
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                // int x = 100 < e.motion.x && e.motion.x < 100 + 600;
                int y = 170 < e.motion.y && e.motion.y < 170 + 260;
                int xl = 100 < e.motion.x && e.motion.x < 100 + 600 / 2;
                int xr = 600 / 2 < e.motion.x && e.motion.x < 100 + 600;
                input.left_pad = xl && y;
                input.right_pad = xr && y;
                for (int i = 0; i < left_buttons_size; i++){
                    btn = &left_buttons[i];
                    p.x = e.motion.x - btn->offset.x;
                    p.y = e.motion.y;
                    btn->hover = SDL_PointInRect(&p, &btn->rect);
                }
                for (int i = 0; i < right_buttons_size; i++)
                {
                    btn = &right_buttons[i];
                    p.x = e.motion.x - btn->offset.x;
                    p.y = e.motion.y;
                    btn->hover = SDL_PointInRect(&p, &btn->rect);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&mouse_point.x, &mouse_point.y);
                // debug_printf("Click [%d, %d]\n", mouse_point.x, mouse_point.y);
                for (int i = 0; i < left_buttons_size; i++)
                {
                    btn = &left_buttons[i];
                    p.x = mouse_point.x - btn->offset.x;
                    p.y = mouse_point.y;
                    if (SDL_PointInRect(&p, &btn->rect))
                    {
                        btn->enabled = (btn->toggle) ? !btn->enabled : btn->enabled;

                        if (btn->icon == ICON_SETTINGS)
                        {
                            deck_input->settings = !deck_input->settings;
                        }
                        if (deck_input->events == nullptr)
                            break;
                        
                        f.type = SDL_KEYDOWN;
                        f.key.keysym.sym = btn->key;
                        f.key.keysym.scancode = btn->scan;
                        deck_input->events(&f, deck_input);
                        break;
                    }
                }
                for (int i = 0; i < right_buttons_size; i++)
                {
                    btn = &right_buttons[i];
                    
                    p.x = mouse_point.x - btn->offset.x;
                    p.y = mouse_point.y;
                    if (SDL_PointInRect(&p, &btn->rect))
                    {
                        btn->enabled = (btn->toggle) ? !btn->enabled : true;

                        if (btn->icon == ICON_SETTINGS)
                        {
                            deck_input->settings = !deck_input->settings;
                        }
                        if (deck_input->events == nullptr)
                            break;

                        f.type = SDL_KEYDOWN;
                        f.key.keysym.sym = btn->key;
                        f.key.keysym.scancode = btn->scan;
                        debug_printf("Key Down %s %d %d\n",btn->icon, btn->key, btn->scan);
                        deck_input->events(&f, deck_input);
                        break;
                    }
                }
                if (!deck_input->settings)
                    break;
                SettingMenuOption_t *opt;
                for (int i = 0; i < settings_count; i++)
                {
                    opt = &settings[i];
                    if (SDL_PointInRect(&mouse_point, &opt->rect))
                    {
                        opt->enabled = !opt->enabled; // Toggle state

                        if (opt == BigButton)
                        {
                            fonts.icon = (BigButton->enabled) ? fonts.icon48 : fonts.icon32;
                            BUTTON_SIZE = (BigButton->enabled) ? BUTTON_FONT_SIZE_MEDIUM : BUTTON_FONT_SIZE_SMALL;
                            resize_buttons(right_buttons, right_buttons_size, BUTTON_SIZE);
                            resize_buttonsleft(left_buttons, left_buttons_size, BUTTON_SIZE);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&mouse_point.x, &mouse_point.y);
                for (int i = 0; i < right_buttons_size; i++)
                {
                    btn = &right_buttons[i];
                    //int offset = (BigButton->enabled) ? btn->offset.x * 2 : btn->offset.x;
                    p.x = mouse_point.x - btn->offset.x;
                    p.y = mouse_point.y;
                    if (SDL_PointInRect(&p, &btn->rect))
                    {
                        btn->enabled = (btn->toggle) ? btn->enabled : false;

                        if (deck_input->events == nullptr)
                            break;
                        f.type = SDL_KEYUP;
                        f.key.keysym.sym = btn->key;
                        f.key.keysym.scancode = btn->scan;
                        debug_printf("Key UP %s %d %d\n", btn->icon, btn->key, btn->scan );
                        deck_input->events(&f, deck_input);
                        break;
                    }
                }
            case SDL_CONTROLLERAXISMOTION:
                // event.caxis.value ranges from -32768 to 32767
                if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
                {
                    input.left_stick = leftStick != e.caxis.value && e.caxis.value != 0;
                    leftStick = e.caxis.value;
                }
                if (e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX || e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
                {
                    input.right_stick = rightStick != e.caxis.value && e.caxis.value != 0;
                    rightStick = e.caxis.value;
                }
                break;
            case SDL_CONTROLLERDEVICEADDED:
                switch (e.cdevice.type)
                {
                case SDL_CONTROLLERDEVICEADDED:
                    controllers_list_add(&root, e.cdevice.which, SDL_GameControllerOpen(e.cdevice.which));
                    break;
                case SDL_CONTROLLERDEVICEREMOVED:
                    controllers_list_remove(&root, e.cdevice.which);
                    break;
                }
                break;
            case SDL_WINDOWEVENT:
                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    winW = e.window.data1;
                    winH = e.window.data2;
                    for (int i = 0; i < right_buttons_size; i++)
                    {
                        btn = &right_buttons[i];
                        btn->rect.x = winW;
                    }
                    break;
                default:
                    break;
                }

            default:
                break;
            }
        }

        // Set the window fullscreen state
        if (SDL_SetWindowFullscreen(win, FullScreen->enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) < 0)
        {
            // Handle error (optional)
            SDL_Log("Could not toggle fullscreen: %s", SDL_GetError());
        }
        
        //usleep(1);
        //SDL_Delay(1);  // Save cpu
        
        if(SDL_GetTicks() < next_render_time){ continue; }
        next_render_time += FRAME_DELAY; 
        
        if (deck_input->settings)
        {
            // Settings
            SDL_RenderClear(render);
            draw_settings(render, settings, settings_count, &fonts);
        }
        else if(ShowDeck->enabled)
        {
            // Render to High-Res Canvas
            SDL_SetRenderTarget(render, canvas);
            SDL_SetRenderDrawColor(render, 20, 20, 20, 255);
            SDL_RenderClear(render);

            drawSteamDeck(render, 0,0, scale, &input);

            if (GridMenuItem->enabled)
                DrawGrid(render, winW, winH, scale);

            //  Downscale to Screen
            SDL_SetRenderTarget(render, NULL);
            SDL_RenderClear(render);
            SDL_RenderCopy(render, canvas, NULL, NULL);

            
        }else {
            SDL_SetRenderTarget(render, canvas);
            SDL_SetRenderDrawColor(render, 20, 20, 20, 255);
            SDL_RenderClear(render);

            if (GridMenuItem->enabled)
                DrawGrid(render, winW, winH, scale);
            SDL_SetRenderTarget(render, NULL);
            SDL_RenderClear(render);
            SDL_RenderCopy(render, canvas, NULL, NULL);
    

            //renderText(render, fonts.icon, deck_input->ready ? ICON_BLUETOOTH_CONNECTED : ICON_BLUETOOTH, SDL_COLOR_WHITE, &button_bluetooth.rect);
        }
        int padding = 10;
        int offset;
        // = (BigButton->enabled) ? button_bluetooth.offset.x  : button_bluetooth.offset.x;
        // //renderText(render, fonts.icon, deck_input->ready ? ICON_BLUETOOTH_CONNECTED : ICON_BLUETOOTH, SDL_COLOR_WHITE, &button_bluetooth.rect);
        // if (ButtonBox->enabled){
        //     SDL_Rect outlineRect = {button_bluetooth.rect.x + offset - (padding / 2), button_bluetooth.rect.y, button_bluetooth.rect.w + padding, button_bluetooth.rect.h + padding}; // x, y, width, height
        //     DrawRoundedBoxWireframe(render, outlineRect, 6, SDL_COLOR_WHITE);
        //     DrawRoundedBox(render, outlineRect, 6, deck_input->settings?  SDL_COLOR_GRAY : SDL_COLOR_LIGHT_GRAY);
        //     SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
        // }
        // SDL_Rect offsetRect = {button_bluetooth.rect.x + offset, button_bluetooth.rect.y, button_bluetooth.rect.w, button_bluetooth.rect.h}; // x, y, width, height
        // renderText(render, fonts.icon, deck_input->ready ? ICON_BLUETOOTH_CONNECTED : ICON_BLUETOOTH, SDL_COLOR_WHITE, &offsetRect);

        for (int i = 0; i < left_buttons_size; i++)
        {
            btn = &left_buttons[i];
            // SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
            
            offset = (BigButton->enabled) ? btn->offset.x  : btn->offset.x;
            if (ButtonBox->enabled){
                SDL_Rect outlineRect = {btn->rect.x + offset - (padding / 2), btn->rect.y, btn->rect.w + padding, btn->rect.h + padding}; // x, y, width, height
                DrawRoundedBoxWireframe(render, outlineRect, 6, SDL_COLOR_WHITE);
                DrawRoundedBox(render, outlineRect, 6, deck_input->settings?  SDL_COLOR_GRAY : SDL_COLOR_LIGHT_GRAY);
                //SDL_RenderDrawRect(render, &outlineRect);
                SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
            }

            SDL_Rect offsetRect = {btn->rect.x + offset, btn->rect.y, btn->rect.w, btn->rect.h}; // x, y, width, height
            renderText(render, fonts.icon, btn->enabled ?  btn->icon_enabled : btn->icon , btn->hover ? SDL_COLOR_BLUE : SDL_COLOR_WHITE, &offsetRect);
        }

        
        for (int i = 0; i < right_buttons_size; i++)
        {
            btn = &right_buttons[i];
            // SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
            
            offset = (BigButton->enabled) ? btn->offset.x  : btn->offset.x;
            if (ButtonBox->enabled){
                SDL_Rect outlineRect = {btn->rect.x + offset - (padding / 2), btn->rect.y, btn->rect.w + padding, btn->rect.h + padding}; // x, y, width, height
                DrawRoundedBoxWireframe(render, outlineRect, 6, SDL_COLOR_WHITE);
                DrawRoundedBox(render, outlineRect, 6, deck_input->settings?  SDL_COLOR_GRAY : SDL_COLOR_LIGHT_GRAY);
                //SDL_RenderDrawRect(render, &outlineRect);
                SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
            }

            SDL_Rect offsetRect = {btn->rect.x + offset, btn->rect.y, btn->rect.w, btn->rect.h}; // x, y, width, height
            renderText(render, fonts.icon, btn->icon, btn->hover ? SDL_COLOR_BLUE : SDL_COLOR_WHITE, &offsetRect);
        }

        SDL_RenderPresent(render);

        #ifdef __FRAME_DELAY__
        
        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
        #endif

    }
    fonts_close(&fonts);
    controllers_list_delete(&root);
    SDL_DestroyTexture(canvas);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
