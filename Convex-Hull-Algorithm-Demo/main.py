from tkinter import *
import random
import time
import math

CanvasSIZE = 600
CanvasBORDER = 10
SleepTIME = 0.1
PointSIZE = 30
global points


def generateJarvisMarch():
    global points
    canvas.delete("all")
    draw_points(points, "black")
    jarvisMarchConvexHull(points)


def generateGraham():
    global points
    canvas.delete("all")
    draw_points(points, "black")
    grahamConvexHull(points)


def generateChan():
    global points
    canvas.delete("all")
    draw_points(points, "black")
    chanConvexHull(points)


def refresh():
    global points
    canvas.delete("all")
    points = []
    for i in range(PointSIZE):
        points.append((random.randint(CanvasBORDER, CanvasSIZE - CanvasBORDER),
                       random.randint(CanvasBORDER, CanvasSIZE - CanvasBORDER)))
    draw_points(points, "black")


def draw_points(points, color):
    for p in points:
        p = (p[0], CanvasSIZE - p[1])
        point = canvas.create_oval(p[0], p[1], p[0], p[1], w=2, outline=color)
    return point


def draw_point(point, color, permanent):
    point = (point[0], CanvasSIZE - point[1])
    point = canvas.create_oval(point[0], point[1], point[0], point[1], w=2, outline=color)
    if permanent is False:
        time.sleep(SleepTIME)
        canvas.delete(point)
        root.update()
    return point


def draw_line(p1, p2, color, permanent):
    p1 = (p1[0], CanvasSIZE - p1[1])
    p2 = (p2[0], CanvasSIZE - p2[1])
    line = canvas.create_line(p1, p2, fill=color)
    root.update()
    if permanent is False:
        time.sleep(SleepTIME)
        canvas.delete(line)
        root.update()
    return line


def draw_polygon(points, fill, outline):
    if len(points) > 1:
        new_points = []
        for p in points:
            new_points.append((p[0], CanvasSIZE - p[1]))
        canvas.create_polygon(new_points, fill=fill, outline=outline)





def grahamConvexHull(points):
    def slope(p1, p2):
        return 1.0 * (p1[1] - p2[1]) / (p1[0] - p2[0]) if p1[0] != p2[0] else float('inf')

    def cross(pointA, pointB, pointC):
        draw_line(pointA, pointB, "red", False)
        return (pointA[0] * pointB[1] +
                pointC[0] * pointA[1] +
                pointB[0] * pointC[1] -
                pointC[0] * pointB[1] -
                pointB[0] * pointA[1] -
                pointA[0] * pointC[1])

    start = min(points, key=lambda p: (p[0], p[1]))
    points.pop(points.index(start))

    points.sort(key=lambda p: (slope(p, start), -p[1], p[0]))

    line = []
    ans = [start]
    for p in points:
        ans.append(p)
        while len(ans) > 2 and cross(ans[-3], ans[-2], ans[-1]) < 0:
            ans.pop(-2)
            canvas.delete(line.pop())
        if (len(ans) >= 2):
            line.append(draw_line(ans[-2], ans[-1], "red", True))
    draw_polygon(ans, '', 'black')
    return ans


def jarvisMarchConvexHull(points):
    def orientation(p, q, r):
        val = (q[1] - p[1]) * (r[0] - q[0]) - (q[0] - p[0]) * (r[1] - q[1])

        if val == 0:
            return 0
        elif val > 0:
            return 1
        else:
            return 2

    def Left_index(points):
        minn = 0
        for i in range(1, len(points)):
            if points[i][0] < points[minn][0]:
                minn = i
            elif points[i][0] == points[minn][0]:
                if points[i][1] > points[minn][1]:
                    minn = i
        return minn

    n = len(points)
    if n <= 3:
        return points
    hull = []
    l = Left_index(points)
    p = l
    q = 0
    while True:
        hull.append(points[p])
        draw_point(points[p], "red", True)
        q = (p + 1) % n
        lines = []
        for i in range(n):
            lines.append(draw_line(points[p], points[i], "green", True))
            if (orientation(points[p],
                            points[i], points[q]) == 2):
                q = i
        time.sleep(SleepTIME)
        for line in lines:
            canvas.delete(line)
        draw_line(points[p], points[q], "red", True)

        p = q
        if p == l:
            break

    draw_polygon(hull, '', 'black')


def chanConvexHull(points):
    points.sort()
    point1 = points[0:math.floor(1 / 4 * PointSIZE)]
    point2 = points[math.floor(1 / 4 * PointSIZE):math.floor(2 / 4 * PointSIZE)]
    point3 = points[math.floor(2 / 4 * PointSIZE):math.floor(3 / 4 * PointSIZE)]
    point4 = points[math.floor(3 / 4 * PointSIZE):]
    result1 = grahamConvexHull(point1)
    result2 = grahamConvexHull(point2)
    result3 = grahamConvexHull(point3)
    result4 = grahamConvexHull(point4)
    jarvisMarchConvexHull(result1 + result2 + result3 + result4)


root = Tk()
root.title("Convex Hull")
root.geometry('900x600')
canvas = Canvas(root, bg="white", height=CanvasSIZE, width=CanvasSIZE)
canvas.pack(side="left")
refresh()
jarvisMarchButton = Button(root, text="Jarvis March", command=generateJarvisMarch)
grahamButton = Button(root, text="Graham's scan", command=generateGraham)
chanButton = Button(root, text="Chan", command=generateChan)
RefreshButton = Button(root, text="Refresh", command=refresh)
grahamButton.pack()
jarvisMarchButton.pack()
chanButton.pack()
RefreshButton.pack()

root.mainloop()
