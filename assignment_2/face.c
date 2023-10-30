#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define MAX_VERTICES 100000
#define MAX_TRIANGLES 100000

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int isVertexMode = 1;
int imageSize = SCREEN_WIDTH;
int imageMidpoint = SCREEN_WIDTH / 2;
double depth = 2.0;
double amplitude = SCREEN_WIDTH;
double sensitivity = 0.005;
SDL_Point pixels[SCREEN_WIDTH][SCREEN_HEIGHT];

int numVertices = 0;
double vertexData[MAX_VERTICES][3];
int numTriangles = 0;
int triangleData[MAX_TRIANGLES][3];

// Add a global variable to store the zoom factor
double zoomFactor = 1.0;

void rotateXAxis(double theta) {
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    for (int i = 0; i < numVertices; i++) {
        double x = vertexData[i][0];
        double y = cosTheta * vertexData[i][1] - sinTheta * vertexData[i][2];
        double z = sinTheta * vertexData[i][1] + cosTheta * vertexData[i][2];
        vertexData[i][0] = x;
        vertexData[i][1] = y;
        vertexData[i][2] = z;
    }
}

void rotateYAxis(double theta) {
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    for (int i = 0; i < numVertices; i++) {
        double x = vertexData[i][2] * sinTheta + vertexData[i][0] * cosTheta;
        double y = vertexData[i][1];
        double z = -sinTheta * vertexData[i][0] + cosTheta * vertexData[i][2];
        vertexData[i][0] = x;
        vertexData[i][1] = y;
        vertexData[i][2] = z;
    }
}

void loadVerticesAndTriangles() {
    FILE *verticesFile = fopen("face-vertices.data", "r");
    if (verticesFile == NULL) {
        printf("Error opening vertices file\n");
        exit(1);
    }

    while (fscanf(verticesFile, "%lf,%lf,%lf", &vertexData[numVertices][0], &vertexData[numVertices][1], &vertexData[numVertices][2]) == 3) {
        numVertices++;
    }

    fclose(verticesFile);

    FILE *trianglesFile = fopen("face-index.txt", "r");
    if (trianglesFile == NULL) {
        printf("Error opening triangles file\n");
        exit(1);
    }

    while (fscanf(trianglesFile, "%d,%d,%d", &triangleData[numTriangles][0], &triangleData[numTriangles][1], &triangleData[numTriangles][2]) == 3) {
        numTriangles++;
    }

    fclose(trianglesFile);
}

void resetPixels() {
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            pixels[i][j].x = 0;
            pixels[i][j].y = 0;
        }
    }
}

void drawPixel(int x, int y) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        pixels[x][y].x = 255;
        pixels[x][y].y = 255;
        pixels[x][y].y = 255;
    }
}

void plotVertices() {
    for (int i = 0; i < numVertices; i++) {
        double zNormalized = vertexData[i][2] / depth;
        int xPixel = (int)(amplitude * vertexData[i][0] / (1.0 + zNormalized));
        int yPixel = (int)(amplitude * vertexData[i][1] / (1.0 - zNormalized));
        drawPixel(imageMidpoint + xPixel, imageMidpoint - yPixel);
    }
}

void plotTriangles() {
    for (int i = 0; i < numTriangles; i++) {
        int vertexA = triangleData[i][0];
        int vertexB = triangleData[i][1];
        int vertexC = triangleData[i][2];

        double zNormalizedA = vertexData[vertexA][2] / depth;
        int xPixelA = (int)(amplitude * vertexData[vertexA][0] / (1.0 + zNormalizedA));
        int yPixelA = (int)(amplitude * vertexData[vertexA][1] / (1.0 - zNormalizedA));

        double zNormalizedB = vertexData[vertexB][2] / depth;
        int xPixelB = (int)(amplitude * vertexData[vertexB][0] / (1.0 + zNormalizedB));
        int yPixelB = (int)(amplitude * vertexData[vertexB][1] / (1.0 - zNormalizedB));

        double zNormalizedC = vertexData[vertexC][2] / depth;
        int xPixelC = (int)(amplitude * vertexData[vertexC][0] / (1.0 + zNormalizedC));
        int yPixelC = (int)(amplitude * vertexData[vertexC][1] / (1.0 - zNormalizedC));

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, imageMidpoint + xPixelA, imageMidpoint - yPixelA, imageMidpoint + xPixelB, imageMidpoint - yPixelB);
        SDL_RenderDrawLine(renderer, imageMidpoint + xPixelA, imageMidpoint - yPixelA, imageMidpoint + xPixelC, imageMidpoint - yPixelC);
        SDL_RenderDrawLine(renderer, imageMidpoint + xPixelC, imageMidpoint - yPixelC, imageMidpoint + xPixelB, imageMidpoint - yPixelB);
    }
}

void refreshDisplay() {
    resetPixels();

    if (isVertexMode) {
        plotVertices();
    } else {
        plotTriangles();
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            if (pixels[i][j].x != 0) {
                SDL_SetRenderDrawColor(renderer, pixels[i][j].x, pixels[i][j].y, pixels[i][j].y, 255);
                SDL_RenderDrawPoint(renderer, i, j);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void handleRotationAndZoom(int x, int y) {
    static int prevX = -1;
    static int prevY = -1;

    if (isVertexMode) {
        int rotationThreshold = 40;
        if (prevX != -1) {
            int xDiff = prevX - x;
            int yDiff = prevY - y;
            if (abs(xDiff) + abs(yDiff) > rotationThreshold) {
                rotateXAxis(-sensitivity * yDiff);
                rotateYAxis(-sensitivity * xDiff);
                refreshDisplay();
                prevX = x;
                prevY = y;
            }
        } else {
            prevX = x;
            prevY = y;
        }
    } else {
        int zoomThreshold = 60;
        if (prevY != -1) {
            int yDiff = prevY - y;
            if (abs(yDiff) > zoomThreshold / 3) {
                // Zoom in or out based on yDiff
                zoomFactor *= exp(0.005 * yDiff);
                amplitude = SCREEN_WIDTH * zoomFactor;
                refreshDisplay();
                prevY = y;
            }
        } else {
            prevY = y;
        }
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("3D Object Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (window == NULL || renderer == NULL) {
        fprintf(stderr, "SDL_CreateWindow and SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    loadVerticesAndTriangles();
    rotateXAxis(-15 * M_PI / 180);
    rotateYAxis(-30 * M_PI / 180);

    refreshDisplay();

    SDL_Event event;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;

                case SDL_MOUSEMOTION:
                    handleRotationAndZoom(event.motion.x, event.motion.y);
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
                        // Zoom in
                        zoomFactor *= 1.1; // Adjust the zoom factor as needed
                        amplitude = SCREEN_WIDTH * zoomFactor;
                        refreshDisplay();
                    } else if (event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_KP_MINUS) {
                        // Zoom out
                        zoomFactor *= 0.9; // Adjust the zoom factor as needed
                        amplitude = SCREEN_WIDTH * zoomFactor;
                        refreshDisplay();
                    }
                    break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

