#include "raylib.h"
#include <stdbool.h>
#include <stdio.h> // NEW: Required for file save/load operations

int board[64] = {0};
int difficulty = 1; // 0 = Easy, 1 = Hard (Defaults to Hard)
bool tosAccepted = false;
bool showTutorial = false; // Tracks if the instructions are being viewed

const int WIN_LINES[76][4] = {
    // Horizontal Rows (Layers 0-3)
    {0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15},
    {16, 17, 18, 19}, {20, 21, 22, 23}, {24, 25, 26, 27}, {28, 29, 30, 31},
    {32, 33, 34, 35}, {36, 37, 38, 39}, {40, 41, 42, 43}, {44, 45, 46, 47},
    {48, 49, 50, 51}, {52, 53, 54, 55}, {56, 57, 58, 59}, {60, 61, 62, 63},
    // Vertical Columns (Layers 0-3)
    {0, 4, 8, 12}, {1, 5, 9, 13}, {2, 6, 10, 14}, {3, 7, 11, 15},
    {16, 20, 24, 28}, {17, 21, 25, 29}, {18, 22, 26, 30}, {19, 23, 27, 31},
    {32, 36, 40, 44}, {37, 41, 45, 49}, {34, 38, 42, 46}, {35, 39, 43, 47},
    {48, 52, 56, 60}, {49, 53, 57, 61}, {50, 54, 58, 62}, {51, 55, 59, 63},
    // Vertical Pillars
    {0, 16, 32, 48}, {1, 17, 33, 49}, {2, 18, 34, 50}, {3, 19, 35, 51},
    {4, 20, 36, 52}, {5, 21, 37, 53}, {6, 22, 38, 54}, {7, 23, 39, 55},
    {8, 24, 40, 56}, {9, 25, 41, 57}, {10, 26, 42, 58}, {11, 27, 43, 59},
    {12, 28, 44, 60}, {13, 29, 45, 61}, {14, 30, 46, 62}, {15, 31, 47, 63},
    // Flat Diagonals
    {0, 5, 10, 15}, {3, 6, 9, 12},
    {16, 21, 26, 31}, {19, 22, 25, 28},
    {32, 37, 42, 47}, {35, 38, 41, 44},
    {48, 53, 58, 63}, {51, 54, 57, 60},
    // Deep Spacial Diagonals
    {0, 21, 42, 63}, {3, 22, 41, 60}, {12, 25, 38, 51}, {15, 26, 37, 48}
};

// NEW: Helper function to save current board and difficulty data to disk
void SaveGame(void) {
    FILE *file = fopen("savegame.dat", "wb");
    if (file != NULL) {
        fwrite(board, sizeof(int), 64, file);
        fwrite(&difficulty, sizeof(int), 1, file);
        int tosSaved = tosAccepted ? 1 : 0;
        fwrite(&tosSaved, sizeof(int), 1, file);
        fclose(file);
    }
}

// NEW: Helper function to load progress dynamically on startup
void LoadGame(void) {
    FILE *file = fopen("savegame.dat", "rb");
    if (file != NULL) {
        fread(board, sizeof(int), 64, file);
        fread(&difficulty, sizeof(int), 1, file);
        int tosSaved = 0;
        fread(&tosSaved, sizeof(int), 1, file);
        tosAccepted = (tosSaved == 1);
        fclose(file);
    }
}

Vector3 GetCellPosition(int index, float spacing) {
    int x = index % 4;
    int y = (index / 4) % 4;
    int z = index / 16;

    float posX = (x - 1.5f) * 2.0f;
    float posY = (z - 1.5f) * spacing; 
    float posZ = (y - 1.5f) * 2.0f;

    return (Vector3){ posX, posY, posZ };
}

void DrawX3D(Vector3 pos, float size, Color color) {
    float h = size / 2.0f;
    DrawLine3D((Vector3){pos.x - h, pos.y + h, pos.z}, (Vector3){pos.x + h, pos.y - h, pos.z}, color);
    DrawLine3D((Vector3){pos.x + h, pos.y + h, pos.z}, (Vector3){pos.x - h, pos.y - h, pos.z}, color);
}

void DrawO3D(Vector3 pos, float radius, Color color) {
    DrawCircle3D(pos, radius, (Vector3){0, 0, 1}, 0.0f, color);
}

int check_win() {
    for (int i = 0; i < 76; i++) {
        int p1 = board[WIN_LINES[i][0]];
        int p2 = board[WIN_LINES[i][1]];
        int p3 = board[WIN_LINES[i][2]];
        int p4 = board[WIN_LINES[i][3]];
        if (p1 != 0 && p1 == p2 && p2 == p3 && p3 == p4) return i;
    }
    return -1;
}

int GetAIMove(int *board) {
    for (int p = 0; p < 2; p++) {
        int targetPlayer = (p == 0) ? 2 : 1;

        for (int l = 0; l < 4; l++) {
            for (int r = 0; r < 4; r++) {
                int count = 0, emptyIdx = -1;
                for (int c = 0; c < 4; c++) {
                    int idx = (l * 16) + (r * 4) + c;
                    if (board[idx] == targetPlayer) count++;
                    else if (board[idx] == 0) emptyIdx = idx;
                }
                if (count == 3 && emptyIdx != -1) return emptyIdx;
            }
            for (int c = 0; c < 4; c++) {
                int count = 0, emptyIdx = -1;
                for (int r = 0; r < 4; r++) {
                    int idx = (l * 16) + (r * 4) + c;
                    if (board[idx] == targetPlayer) count++;
                    else if (board[idx] == 0) emptyIdx = idx;
                }
                if (count == 3 && emptyIdx != -1) return emptyIdx;
            }
            int countD1 = 0, emptyD1 = -1;
            int countD2 = 0, emptyD2 = -1;
            for (int i = 0; i < 4; i++) {
                int idx1 = (l * 16) + (i * 4) + i;
                if (board[idx1] == targetPlayer) countD1++;
                else if (board[idx1] == 0) emptyD1 = idx1;

                int idx2 = (l * 16) + (i * 4) + (3 - i);
                if (board[idx2] == targetPlayer) countD2++;
                else if (board[idx2] == 0) emptyD2 = idx2;
            }
            if (countD1 == 3 && emptyD1 != -1) return emptyD1;
            if (countD2 == 3 && emptyD2 != -1) return emptyD2;
        }

        for (int i = 0; i < 16; i++) {
            int count = 0, emptyIdx = -1;
            for (int l = 0; l < 4; l++) {
                int idx = (l * 16) + i;
                if (board[idx] == targetPlayer) count++;
                else if (board[idx] == 0) emptyIdx = idx;
            }
            if (count == 3 && emptyIdx != -1) return emptyIdx;
        }

        for (int i = 0; i < 4; i++) {
            int countF1 = 0, emptyF1 = -1;
            int countF2 = 0, emptyF2 = -1;
            int countS1 = 0, emptyS1 = -1;
            int countS2 = 0, emptyS2 = -1;
            for (int j = 0; j < 4; j++) {
                int idxF1 = (j * 16) + (j * 4) + i;
                if (board[idxF1] == targetPlayer) countF1++;
                else if (board[idxF1] == 0) emptyF1 = idxF1;

                int idxF2 = (j * 16) + ((3 - j) * 4) + i;
                if (board[idxF2] == targetPlayer) countF2++;
                else if (board[idxF2] == 0) emptyF2 = idxF2;

                int idxS1 = (j * 16) + (i * 4) + j;
                if (board[idxS1] == targetPlayer) countS1++;
                else if (board[idxS1] == 0) emptyS1 = idxS1;

                int idxS2 = (j * 16) + (i * 4) + (3 - j);
                if (board[idxS2] == targetPlayer) countS2++;
                else if (board[idxS2] == 0) emptyS2 = idxS2;
            }
            if (countF1 == 3 && emptyF1 != -1) return emptyF1;
            if (countF2 == 3 && emptyF2 != -1) return emptyF2;
            if (countS1 == 3 && emptyS1 != -1) return emptyS1;
            if (countS2 == 3 && emptyS2 != -1) return emptyS2;
        }

        int corners[4][4] = {
            {0, 21, 42, 63},
            {3, 22, 41, 60},
            {12, 25, 38, 51},
            {15, 26, 37, 48}
        };
        for (int d = 0; d < 4; d++) {
            int count = 0, emptyIdx = -1;
            for (int i = 0; i < 4; i++) {
                int idx = corners[d][i];
                if (board[idx] == targetPlayer) count++;
                else if (board[idx] == 0) emptyIdx = idx;
            }
            if (count == 3 && emptyIdx != -1) return emptyIdx;
        }
    }

    int strategicOrder[64] = {
        21, 22, 25, 26, 37, 38, 41, 42,
        0, 3, 12, 15, 48, 51, 60, 63 
    };
    for (int i = 0; i < 16; i++) {
        int cell = strategicOrder[i];
        if (board[cell] == 0) return cell;
    }

    for (int i = 0; i < 64; i++) {
        if (board[i] == 0) return i;
    }
    return -1;
}

int main(void) {
    int currentPlayer = 1;
    InitWindow(1200, 800, "3D Qubic - Matrix Edition");
 
    Camera3D camera = { 0 };
    camera.position = (Vector3){ -14.2f, 8.0f, -5.0f }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    bool mergedMode = false;
    float currentSpacing = 2.2f; 
    
    SetTargetFPS(60);

    // NEW: Auto-load saved matrix status right on boot
    LoadGame();

    while (!WindowShouldClose())
    {
        // Guard Active: Stops game inputs entirely if overlay is up
        if (!tosAccepted)
        {
            goto skip_game_logic;
        }

        int winIndex = check_win();

        if (IsKeyPressed(KEY_SPACE)) mergedMode = !mergedMode;

        if (IsKeyPressed(KEY_R)) {
            for (int i = 0; i < 64; i++) {
                board[i] = 0;
            }
            currentPlayer = 1;
            winIndex = -1;
            SaveGame(); // Save empty layout state
        }
    
        float targetSpacing = mergedMode ? 1.5f : 2.2f; 
        currentSpacing += (targetSpacing - currentSpacing) * 0.1f; 
        if (IsKeyDown(KEY_LEFT)) camera.position.x -= 0.1f;
        if (IsKeyDown(KEY_RIGHT)) camera.position.x += 0.1f;
    
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();

            if (mousePos.x >= 40 && mousePos.x <= 180 &&
                mousePos.y >= 510 && mousePos.y <= 545) {

                for (int i = 0; i < 64; i++) board[i] = 0;
                currentPlayer = 1;
                winIndex = -1;
                SaveGame(); // Save clean board state
            }
            else if (mousePos.x >= 40 && mousePos.x <= 180 &&
                     mousePos.y >= 555 && mousePos.y <= 590) 
            {
                difficulty = (difficulty + 1) % 4;
                SaveGame(); // Save modified difficulty state
            }
            else if (winIndex == -1 && currentPlayer == 1) {
                int boardSize = 80;
                int startX = 40;
                int cellSize = boardSize / 4;

                for (int gridLayer = 0; gridLayer < 4; gridLayer++) {
                    int visualRow = 3 - gridLayer;
                    int startY = 50 + (visualRow * 115);

                    if (mousePos.x >= startX && mousePos.x < startX + boardSize &&
                        mousePos.y >= startY && mousePos.y < startY + boardSize) {

                        int col = (mousePos.x - startX) / cellSize;
                        int row = (mousePos.y - startY) / cellSize;
                        int cellIndex = (gridLayer * 16) + (row * 4) + col;

                        if (board[cellIndex] == 0) {
                            board[cellIndex] = 1;
                            winIndex = check_win();

                            if (winIndex == -1) {
                                int aiMove = -1;
                                int mistakeChance = 0;

                                if (difficulty == 0) mistakeChance = 60;
                                else if (difficulty == 1) mistakeChance = 30;
                                else if (difficulty == 2) mistakeChance = 10;
                                else if (difficulty == 3) mistakeChance = 0;

                                if (GetRandomValue(1, 100) <= mistakeChance) {
                                    int emptySlots[64];
                                    int count = 0;
                                    for (int i = 0; i < 64; i++) {
                                        if (board[i] == 0) emptySlots[count++] = i;
                                    }
                                    if (count > 0) aiMove = emptySlots[GetRandomValue(0, count - 1)];
                                } else {
                                    aiMove = GetAIMove(board);
                                }

                                if (aiMove != -1) {
                                    board[aiMove] = 2;
                                    winIndex = check_win();
                                }
                            }
                            SaveGame(); // Save state after turns take place
                        }
                    }
                }
            }
        }

skip_game_logic:

        BeginDrawing();
            ClearBackground(BLACK); 

            BeginMode3D(camera);
                winIndex = check_win();
    
                Vector2 layerLabelPositions[4];

                for (int layer = 0; layer < 4; layer++) { 
                    float posY = (layer - 1.5f) * currentSpacing;
                    Vector3 layerCenter = { 0.0f, posY, 0.0f };
                    DrawCubeWires(layerCenter, 6.8f, 0.0f, 6.8f, GRAY);

                    Vector3 labelWorldPos = { 3.8f, posY, 0.0f }; 
                    layerLabelPositions[layer] = GetWorldToScreen(labelWorldPos, camera);
                }

                for (int i = 0; i < 64; i++) {
                    Vector3 pos = GetCellPosition(i, currentSpacing);
                    if (board[i] == 1) {
                        DrawX3D(pos, 0.3f, WHITE);
                    } else if (board[i] == 2) {
                        DrawO3D(pos, .15f, RED);
                    } else {
                        DrawCubeWires(pos, 0.2f, 0.0f, 0.2f, DARKGRAY); 
                    }
                }

                if (winIndex != -1) {
                    Vector3 startPt = GetCellPosition(WIN_LINES[winIndex][0], currentSpacing);
                    Vector3 endPt   = GetCellPosition(WIN_LINES[winIndex][3], currentSpacing);
                    DrawLine3D(startPt, endPt, GREEN);
                }
            EndMode3D(); 

            // ---- DRAW 2D STACKED INTERACTIVE LAYERS ----
            int boardSize = 80; 
            int startX = 40;
            int cellSize = boardSize / 4;
            const char* flatLabels[4] = { "BOTTOM", "MID-LOW", "MID-TOP", "TOP" };

            for (int gridLayer = 0; gridLayer < 4; gridLayer++) {
                int visualRow = 3 - gridLayer;
                int startY = 50 + (visualRow * 115);

                DrawText(flatLabels[gridLayer], startX, startY - 14, 10, LIGHTGRAY);

                for (int r = 0; r < 4; r++) {
                    for (int c = 0; c < 4; c++) {
                        int x = startX + (c * cellSize);
                        int y = startY + (r * cellSize);
                        int idx = (gridLayer * 16) + (r * 4) + c;

                        DrawRectangleLines(x, y, cellSize, cellSize, DARKGRAY);

                        if (board[idx] == 1) {
                            DrawLine(x + 3, y + 3, x + cellSize - 3, y + cellSize - 3, WHITE);
                            DrawLine(x + cellSize - 3, y + 3, x + 3, y + cellSize - 3, WHITE);
                        } else if (board[idx] == 2) {
                            DrawCircleLines(x + cellSize/2, y + cellSize/2, cellSize/2 - 3, RED);
                        }
                    }
                }
                DrawRectangleLines(startX, startY, boardSize, boardSize, GRAY);
            }

            // ---- DRAW 2D VISUAL RESET BUTTON ----
            int btnX = 40;
            int btnY = 510; 
            int btnWidth = 140;
            int btnHeight = 35;

            Vector2 currentMouse = GetMousePosition();
            bool isHovered = (currentMouse.x >= btnX && currentMouse.x <= btnX + btnWidth &&
                              currentMouse.y >= btnY && currentMouse.y <= btnY + btnHeight);

            DrawRectangle(btnX, btnY, btnWidth, btnHeight, isHovered ? DARKGRAY : BLACK);
            DrawRectangleLines(btnX, btnY, btnWidth, btnHeight, isHovered ? WHITE : GRAY);
            DrawText("NEW GAME", btnX + 22, btnY + 8, 20, isHovered ? GREEN : LIGHTGRAY);

            // ---- HUD TEXT OVERLAYS ----
            DrawText("Press SPACEBAR to Merge/Separate Layers", 250, 20, 20, LIGHTGRAY);
            DrawText("Use LEFT/RIGHT Arrow Keys to Rotate", 250, 50, 20, GRAY);
            
            if (winIndex == -1) {
                if (currentPlayer == 1) {
                    DrawText("TURN: PLAYER 1 (WHITE X)", 250, 90, 20, WHITE);
                } else {
                    DrawText("TURN: PLAYER 2 (RED O)", 250, 90, 20, RED);
                }
            } else {
                DrawText("GAME OVER!", 250, 90, 20, GOLD);
                if (winIndex == 1) DrawText("PLAYER 1 WINS!", 250, 120, 20, GREEN);
                if (winIndex == 2) DrawText("PLAYER 2 WINS!", 250, 120, 20, GREEN);
            }

            // ---- DRAW 2D VISUAL DIFFICULTY BUTTON ----
            int diffX = 40;
            int diffY = 555; 
            int diffWidth = 140; 
            int diffHeight = 35;

            bool isDiffHovered = (currentMouse.x >= diffX && currentMouse.x <= diffX + diffWidth &&
                                  currentMouse.y >= diffY && currentMouse.y <= diffY + diffHeight);

            DrawRectangle(diffX, diffY, diffWidth, diffHeight, isDiffHovered ? DARKGRAY : BLACK);
            DrawRectangleLines(diffX, diffY, diffWidth, diffHeight, isDiffHovered ? WHITE : GRAY);
            
            if (difficulty == 0) { 
                DrawText("AI: EASY", diffX + 25, diffY + 8, 20, GREEN);
            } else if (difficulty == 1) {
                DrawText("AI: MEDIUM", diffX + 15, diffY + 8, 20, ORANGE);
            } else if (difficulty == 2) {
                DrawText("AI: EXPERT", diffX + 18, diffY + 8, 20, RED);
            } else if (difficulty == 3) {
                DrawText("AI: BRUTAL", diffX + 18, diffY + 8, 20, PURPLE);
            }

            // ---- STARTUP OVERLAY (DYNAMIC TOS & TUTORIAL SWITCH) ----
            if (!tosAccepted) 
            {
                DrawRectangle(0, 0, 1200, 800, Fade(BLACK, 0.85f));

                int boxX = 250;
                int boxY = 120;
                int boxWidth = 700;
                int boxHeight = 560;

                DrawRectangle(boxX, boxY, boxWidth, boxHeight, DARKGRAY);
                DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, WHITE);

                Vector2 overlayMouse = GetMousePosition();

                if (!showTutorial) 
                {
                    // ---- VIEW A: TERMS OF SERVICE ----
                    DrawText("TERMS OF SERVICE & END USER LICENSE AGREEMENT", boxX + 40, boxY + 30, 20, GOLD);
                    DrawLine(boxX + 40, boxY + 65, boxX + 660, boxY + 65, GRAY);

                    int textY = boxY + 100;
                    DrawText("1. LICENSE: Granted for personal entertainment use only.", boxX + 40, textY, 20, WHITE);
                    DrawText("2. OWNERSHIP: Code and AI assets belong to the author.", boxX + 40, textY + 40, 20, WHITE);
                    DrawText("3. LIABILITY: Software is provided 'as-is'.", boxX + 40, textY + 80, 20, WHITE);
                    DrawText("4. PRIVACY: 100% local. No telemetry or data collection.", boxX + 40, textY + 120, 20, WHITE);
                    
                    DrawText("Press 'ACCEPT' to play, or review the game rules below.", boxX + 40, textY + 220, 20, ORANGE);

                    // Button 1: Tutorial Trigger
                    int tutBtnX = boxX + 100;
                    int tutBtnY = boxY + 460;
                    int tutBtnW = 200;
                    int tutBtnH = 45;
                    bool tutHovered = (overlayMouse.x >= tutBtnX && overlayMouse.x <= tutBtnX + tutBtnW &&
                                       overlayMouse.y >= tutBtnY && overlayMouse.y <= tutBtnY + tutBtnH);

                    DrawRectangle(tutBtnX, tutBtnY, tutBtnW, tutBtnH, tutHovered ? LIGHTGRAY : BLACK);
                    DrawRectangleLines(tutBtnX, tutBtnY, tutBtnW, tutBtnH, tutHovered ? BLACK : WHITE);
                    DrawText("HOW TO PLAY", tutBtnX + 45, tutBtnY + 13, 20, tutHovered ? BLACK : WHITE);

                    if (tutHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
                    {
                        showTutorial = true;
                    }

                    // Button 2: Accept License
                    int accBtnX = boxX + 400;
                    int accBtnY = boxY + 460;
                    int accBtnW = 200;
                    int accBtnH = 45;
                    bool accHovered = (overlayMouse.x >= accBtnX && overlayMouse.x <= accBtnX + accBtnW &&
                                       overlayMouse.y >= accBtnY && overlayMouse.y <= accBtnY + accBtnH);

                    DrawRectangle(accBtnX, accBtnY, accBtnW, accBtnH, accHovered ? GREEN : BLACK);
                    DrawRectangleLines(accBtnX, accBtnY, accBtnW, accBtnH, accHovered ? WHITE : GREEN);
                    DrawText("I ACCEPT", accBtnX + 55, accBtnY + 13, 20, accHovered ? BLACK : GREEN);

                    if (accHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
                    {
                        tosAccepted = true;
                        SaveGame(); // Save state immediately upon clicking accept
                    }
                }
                else 
                {
                    // ---- VIEW B: HOW TO PLAY TUTORIAL ----
                    DrawText("3D QUBIC - HOW TO PLAY TUTORIAL", boxX + 40, boxY + 30, 20, SKYBLUE);
                    DrawLine(boxX + 40, boxY + 65, boxX + 660, boxY + 65, GRAY);

                    int textY = boxY + 100;
                    DrawText("THE OBJECTIVE:", boxX + 40, textY, 20, GOLD);
                    DrawText("Get 4 of your markers in a straight line to win!", boxX + 40, textY + 25, 20, WHITE);
                    
                    DrawText("THE 3D MATRIX GRID:", boxX + 40, textY + 70, 20, GOLD);
                    DrawText("- The game is a 4x4x4 cube (64 total positions).", boxX + 40, textY + 95, 20, WHITE);
                    DrawText("- Lines can be horizontal, vertical, or diagonal.", boxX + 40, textY + 120, 20, WHITE);
                    DrawText("- Lines can cut straight through multiple levels!", boxX + 40, textY + 145, 20, WHITE);

                    DrawText("CONTROLS:", boxX + 40, textY + 200, 20, GOLD);
                    DrawText("- LEFT CLICK: Place a piece on the 2D sub-grids.", boxX + 40, textY + 225, 20, WHITE);
                    DrawText("- ARROW KEYS: Rotate camera angle around the cube.", boxX + 40, textY + 250, 20, WHITE);
                    DrawText("- SPACEBAR  : Toggle standard layout / merged view.", boxX + 40, textY + 275, 20, WHITE);
                    DrawText("- 'R' KEY   : Reset the board at any time.", boxX + 40, textY + 300, 20, WHITE);

                    // Button 3: Return to Menu
                    int backX = boxX + 250;
                    int backY = boxY + 470;
                    int backW = 200;
                    int backH = 45;
                    bool backHovered = (overlayMouse.x >= backX && overlayMouse.x <= backX + backW &&
                                        overlayMouse.y >= backY && overlayMouse.y <= backY + backH);

                    DrawRectangle(backX, backY, backW, backH, backHovered ? SKYBLUE : BLACK);
                    DrawRectangleLines(backX, backY, backW, backH, backHovered ? WHITE : SKYBLUE);
                    DrawText("BACK TO MENU", backX + 30, backY + 13, 20, backHovered ? BLACK : SKYBLUE);

                    if (backHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
                    {
                        showTutorial = false;
                    }
                }
            }

        EndDrawing();
    }

    // NEW: Final insurance policy save state right before closing down completely
    SaveGame();

    CloseWindow();
    return 0;
}
