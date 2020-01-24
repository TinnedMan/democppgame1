#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
using namespace std;
// we have to include this because C-out is too slow
//therefore we will be writing to the console buffer directly
#include <Windows.h>
#include <stdio.h>

//set a console width and height

int nScreenWidth = 120;
int nScreenHeight = 40;

//store where player is
float fPlayerX = 14.7f;
float fPlayerY = 5.09f;
//angle player is looking at
float fPlayerA = 0.0f;

//map coords
int nMapHeight = 16;
int nMapWidth = 16;

//field of view

float fFOV = 3.14159f / 4.0f;


//limit of how far to check rays
float fDepth = 16.0f;
//walking speed
float fSpeed = 5.0f;

int main() {

//create a screen buffer
		//create a screen using a wchar instance (new) hence a pointer
		// +1 creates an array of the height and width global variables
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	
	//instance a handle class with a createconsolescreenbuffer method with a regular textmode buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	//crtell buffer it is target of console
	SetConsoleActiveScreenBuffer(hConsole);
	// for the writeconsoleoutputcharacter
	DWORD dwBytesWritten = 0;

	
// creating the map 
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
//game loop

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	while (1)
	{
		// check between two time points
		// get the chrono obj, instantiate and 

		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		// Handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

		// Handle CW Rotation
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (fSpeed * 0.75f) * fElapsedTime;

		// Handle Forwards movement & collision
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		// Handle backwards movement & collision
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}
		for (int x = 0; x < nScreenWidth; x++)
		{
			//for each column, calculate the projected ray angle into world space
			// ERROR !!!float fRayAngle = (fPlayerA - fFOV / 2.0f) * ((float)x / (float)nScreenWidth) * fFOV;
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			//how far is the player from the wall? test thru steping ,
			//until an incrememnt lands in a wall cell
			float fDistanceToWall = 0;
			//
			// ise it the edge of the cell
			bool bHitWall = false;
			//
			bool bBoundary = false;
			float fEyeX = sinf(fRayAngle); //unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);
			//is wall hit?
			//this is the step

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				//increment to step
				fDistanceToWall += 0.1f;

				// test to see if ray hits against length specified above
				// (int) because we are only interested in whole no.
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//if player has crossed a boundary or hit the max on the x or y stop
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					// set to true if true
					bHitWall = true;
					// distance to wall is 16 therefore you've hit maximum distance
					fDistanceToWall = fDepth;
				}// end if
				else {
					//ray is inbounds so test to see if ray cell is a wall
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;

						//test and find four corners of the cell
						vector<pair<float, float>> p; //distance and dot product
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								//perfect integer corner offset from player pos
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								//get magnitude
								float d = sqrt(vx * vx + vy * vy);
								//dot product
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));

							}
						//sort pairs from closest to farthest lambda function
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });
						
						// create boundary
						float fBound = 0.01;
						//2 boundaries
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}//end 2nd if
				}// end else
			}// end while
			
			//calculate distance to ceiling and floor
			// get the midpoint subtract screenheight relative to wall distance
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			//mirror
			int nFloor = nScreenHeight - nCeiling;

			//shader properties
			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588; //close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;
			else										nShade = ' ';	// too far away

			// if boundary

			if (bBoundary)	nShade = ' '; // black the edges 
			
																		 
			//shader 
			for (int y = 0; y < nScreenHeight; y++) 
			{
				if (y <= nCeiling)
					// shade sky as blank space
					screen[y * nScreenWidth + x] = ' ';
				else if(y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				//neither ceiling or floor
				else 
				{	
					//assign characters to appropriate distance
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.25)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';

					screen[y * nScreenWidth + x] = nShade;
				}
			}

		}//end for 

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// Display Map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

		//write to the screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		// windows console, give it buffer, array, coordinate and a pointer to the above dword
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		
	}
	//exit
	return 0;
}