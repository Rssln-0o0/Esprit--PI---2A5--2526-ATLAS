import sys
import qrcode


def generate_qr(data, output_path):
    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=10,
        border=4,
    )
    qr.add_data(data)
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")
    img.save(output_path)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python qr_generator.py <data> <output_path>")
        sys.exit(1)

    data = sys.argv[1]
    output_path = sys.argv[2]
    try:
        generate_qr(data, output_path)
        print(f"QR Code generated at {output_path}")
        sys.exit(0)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
