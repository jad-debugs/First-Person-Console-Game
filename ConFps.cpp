#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <cmath>
using namespace std;

#include <stdio.h>
#include <Windows.h>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 14.7f;
float fPlayerY = 5.09f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159f / 4.0f;
float fDepth = 16.0f;

int main()
{
	// the screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// position on map
	wstring map;
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";


	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	// Updating the Screen until escape sequence
	while (1)
	{
		// this allows us to check te frame rate and attempt to match
		// the system speed with the code where we change character angle view
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// changing character angle view
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (3.0f * 0.75f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (3.0f * 0.75f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned char)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			// checking if we hit a wall
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;;
			}
		}

		if (GetAsyncKeyState((unsigned char)'S') & 0x8000) {

			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;;
			}
		}

		// iterating through columns
		for (int x = 0; x < nScreenWidth; x++) {
			// trying to change angle
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);


			while (!bHitWall && fDistanceToWall < fDepth) {

				fDistanceToWall += 0.1f; //  we continuosly count until we hit something classified as wall

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); // this is how we continously check
				// I had an error here where fEyeY was fEyeX I could not find the reason as to why the game looked so primitive. 
				// If you want try changing it to see the difference. (If only I hadn't copy and pasted the previous line oops!)
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall); 

				// check if we are out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else {
					// of course, if we aren't out, then we are still in the map, so check if ray is wall
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#') {
						bHitWall = true;

						vector<pair<float, float>> p; // the distanec

						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

							// sorting pairs
							sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

							float fBound = 0.01;
							if (acos(p.at(0).second) < fBound) bBoundary = true;
							if (acos(p.at(1).second) < fBound) bBoundary = true;
						}
					}
				}
			}

			// show illusion by making bigger height if further
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;


			short nShade = ' ';

			// depending on distance from wall, color of block will be darker or lighter
			// this handles that
			if (fDistanceToWall <= fDepth / 4.0f)
				nShade = 0x2588;
			else if (fDistanceToWall < fDepth / 3.0f)
				nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)
				nShade = 0x2592;
			else if (fDistanceToWall < fDepth)
				nShade = 0x2591;
			else
				nShade = ' ';

			if (bBoundary)
				nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else {
					//shade the floor based on distance to that pos
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)
						nShade = '#';
					else if (b < 0.5)
						nShade = 'X';
					else if (b < 0.75)
						nShade = '.';
					else if (b < 0.9)
						nShade = '_';
					else
						nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}

		//Display STATS!!!!
		//swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// making a map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

		// to write to the screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

	}

	return 0;
}