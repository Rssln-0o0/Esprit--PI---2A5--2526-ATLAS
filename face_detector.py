
import sys
import os

def main():
    if len(sys.argv) < 2:
        sys.exit(1)
    img_path = sys.argv[1]
    if not os.path.isfile(img_path):
        sys.exit(1)
    try:
        import cv2
    except ImportError:
        sys.exit(1)
    img = cv2.imread(img_path)
    if img is None:
        sys.exit(1)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
    if cascade.empty():
        sys.exit(1)
    faces = cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
    if len(faces) > 0:
        sys.exit(0)
    sys.exit(1)

if __name__ == "__main__":
    main()
