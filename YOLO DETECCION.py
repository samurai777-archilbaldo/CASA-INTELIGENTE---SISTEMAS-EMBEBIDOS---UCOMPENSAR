from ultralytics import YOLO
import cv2

# Cargar tu modelo entrenado
model = YOLO("runs/detect/train/weights/best.pt")

# Abrir cámara (0 = cámara del PC)
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Detectar objetos
    results = model(frame, stream=True)

    for r in results:
        annotated_frame = r.plot()

    # Mostrar en pantalla
    cv2.imshow("Deteccion YOLO", annotated_frame)

    # Salir con tecla 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()