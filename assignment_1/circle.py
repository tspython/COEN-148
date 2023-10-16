from PIL import Image, ImageDraw

width = 320
height = 240
img = Image.new('RGB', (width, height))
draw = ImageDraw.Draw(img)
center = (width // 2, height // 2)
R = 100
color = (255, 0, 0)

# midpoint circle algo
def draw_circle(x0, y0, radius, color):
    x = radius
    y = 0
    err = 0

    while x >= y:
        draw.point((x0 + x, y0 + y), color)
        draw.point((x0 + y, y0 + x), color)
        draw.point((x0 - y, y0 + x), color)
        draw.point((x0 - x, y0 + y), color)
        draw.point((x0 - x, y0 - y), color)
        draw.point((x0 - y, y0 - x), color)
        draw.point((x0 + y, y0 - x), color)
        draw.point((x0 + x, y0 - y), color)

        if err <= 0:
            y += 1
            err += 2 * y + 1

        if err > 0:
            x -= 1
            err -= 2 * x + 1

#draw CIRCLE
draw_circle(center[0], center[1], R, color)
img.save("circle.png")

# Fill the circle #
filled_img = img.copy()
draw = ImageDraw.Draw(filled_img)

def incircle(x, y):
    return (x - center[0]) ** 2 + (y - center[1]) ** 2 <= R ** 2

for x in range(width):
    for y in range(height):
        if incircle(x, y):
            draw.point((x, y), color)

filled_img.save("circle_filled.png")

aa_img = Image.new('RGB', (width, height))
draw_aa = ImageDraw.Draw(aa_img)

# STARRT ANTI-ALIASING #
# using supersampling
def anti_aliasing(x, y, R, color):
    num_samples = 4
    sample_color = [0, 0, 0]

    for i in range(num_samples):
        sample_x = x + (i % 2) - 0.5
        sample_y = y + (i // 2) - 0.5

        if incircle(sample_x, sample_y):
            sample_color[0] += color[0]
            sample_color[1] += color[1]
            sample_color[2] += color[2]

    sample_color[0] //= num_samples
    sample_color[1] //= num_samples
    sample_color[2] //= num_samples

    draw_aa.point((x, y), tuple(sample_color))

for x in range(width):
    for y in range(height):
        anti_aliasing(x, y, R, color)

aa_img.save("circle_aa.png")
