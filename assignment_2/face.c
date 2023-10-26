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

int vert = 1;
int IMGSIZE = SCREEN_WIDTH;
int IMGMID = SCREEN_WIDTH / 2;
double d = 2.0;
double amp = SCREEN_WIDTH;
double sens = 0.005;
SDL_Point pixels[SCREEN_WIDTH][SCREEN_HEIGHT];

int num_vertices = 0;
double vertices[MAX_VERTICES][3];
int num_triangles = 0;
int triangles[MAX_TRIANGLES][3];

void rotateX(double theta) {
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);
    for (int i = 0; i < num_vertices; i++) {
        double x = vertices[i][0];
        double y = cos_theta * vertices[i][1] - sin_theta * vertices[i][2];
        double z = sin_theta * vertices[i][1] + cos_theta * vertices[i][2];
        vertices[i][0] = x;
        vertices[i][1] = y;
        vertices[i][2] = z;
    }
}

void rotateY(double theta) {
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);
    for (int i = 0; i < num_vertices; i++) {
        double x = vertices[i][2] * sin_theta + vertices[i][0] * cos_theta;
        double y = vertices[i][1];
        double z = -sin_theta * vertices[i][0] + cos_theta * vertices[i][2];
        vertices[i][0] = x;
        vertices[i][1] = y;
        vertices[i][2] = z;
    }
}

void loadVerticesAndTriangles() {
    FILE *vertices_file = fopen("face-vertices.data", "r");
    if (vertices_file == NULL) {
        printf("Error opening vertices file\n");
        exit(1);
    }

    while (fscanf(vertices_file, "%lf,%lf,%lf", &vertices[num_vertices][0], &vertices[num_vertices][1], &vertices[num_vertices][2]) == 3) {
        num_vertices++;
    }

    fclose(vertices_file);

    FILE *triangles_file = fopen("face-index.txt", "r");
    if (triangles_file == NULL) {
        printf("Error opening triangles file\n");
        exit(1);
    }

    while (fscanf(triangles_file, "%d,%d,%d", &triangles[num_triangles][0], &triangles[num_triangles][1], &triangles[num_triangles][2]) == 3) {
        num_triangles++;
    }

    fclose(triangles_file);
}

void clean() {
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
    for (int i = 0; i < num_vertices; i++) {
        double zbd = vertices[i][2] / d;
        int xp = (int)(amp * vertices[i][0] / (1.0 + zbd));
        int yp = (int)(amp * vertices[i][1] / (1.0 - zbd));
        drawPixel(IMGMID + xp, IMGMID - yp);
    }
}

void plotTriangles() {
    for (int i = 0; i < num_triangles; i++) {
        int a = triangles[i][0];
        int b = triangles[i][1];
        int c = triangles[i][2];

        double zbda = vertices[a][2] / d;
        int xpa = (int)(amp * vertices[a][0] / (1.0 + zbda));
        int ypa = (int)(amp * vertices[a][1] / (1.0 - zbda));

        double zbdb = vertices[b][2] / d;
        int xpb = (int)(amp * vertices[b][0] / (1.0 + zbdb));
        int ypb = (int)(amp * vertices[b][1] / (1.0 - zbdb));

        double zbdc = vertices[c][2] / d;
        int xpc = (int)(amp * vertices[c][0] / (1.0 + zbdc));
        int ypc = (int)(amp * vertices[c][1] / (1.0 - zbdc));

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, IMGMID + xpa, IMGMID - ypa, IMGMID + xpb, IMGMID - ypb);
        SDL_RenderDrawLine(renderer, IMGMID + xpa, IMGMID - ypa, IMGMID + xpc, IMGMID - ypc);
        SDL_RenderDrawLine(renderer, IMGMID + xpc, IMGMID - ypc, IMGMID + xpb, IMGMID - ypb);
    }
}

void refresh() {
    clean();

    if (vert) {
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

void rotateAndZoom(int x, int y) {
    static int prev_x = -1;
    static int prev_y = -1;
    static double zoomFactor = 1.0;

    if (vert) {
        int threshold = 40;
        if (prev_x != -1) {
            int x_diff = prev_x - x;
            int y_diff = prev_y - y;
            if (abs(x_diff) + abs(y_diff) > threshold) {
                rotateX(-sens * y_diff);
                rotateY(-sens * x_diff);
                refresh();
                prev_x = x;
                prev_y = y;
            }
        } else {
            prev_x = x;
            prev_y = y;
        }
    } else {
        int threshold = 60;
        if (prev_y != -1) {
            int y_diff = prev_y - y;
            if (abs(y_diff) > threshold / 3) {
                zoomFactor *= exp(0.005 * y_diff);
                amp = SCREEN_WIDTH * zoomFactor;
                refresh();
                prev_y = y;
            }
        } else {
            prev_y = y;
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
    rotateX(-15 * M_PI / 180);
    rotateY(-30 * M_PI / 180);

    refresh();

    SDL_Event event;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;

                case SDL_MOUSEMOTION:
                    rotateAndZoom(event.motion.x, event.motion.y);
                    break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

