import xml.etree.ElementTree as ET
import sys

try:
    ET.parse('mainwindow.ui')
    print("XML is valid")
except ET.ParseError as e:
    print(f"XML is invalid: {e}")
    sys.exit(1)
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)
