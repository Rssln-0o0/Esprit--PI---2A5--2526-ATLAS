# -*- coding: utf-8 -*-
from pathlib import Path

bat = Path(r"c:\Users\saida\Desktop\integration\arduino\2a15-smart-fishing-port-management-INTEGR-CRUD-FIN-BAT\mainwindow.ui")
final = Path(r"c:\Users\saida\Desktop\integration\arduino\2a15-smart-fishing-port-management-Integration_Crud_FINAL0001\mainwindow.ui")

bat_lines = bat.read_text(encoding="utf-8").splitlines()
chunk = "\n".join(bat_lines[3552:4660])

text = final.read_text(encoding="utf-8")

btn_anchor = (
 '                <widget class="QPushButton" name="btnSubEmpl5">\n'
    '                 <property name="text">\n'
    '                  <string>Disponibilité</string>\n'
    '                 </property>\n'
    '                 <property name="checkable">\n'
    '                  <bool>true</bool>\n'
    '                 </property>\n'
    '                 <property name="autoExclusive">\n'
    '                  <bool>true</bool>\n'
    '                 </property>\n'
    '                </widget>\n'
    '               </item>\n'
    '               <item>\n'
    '                <spacer name="horizontalSpacer">'
)
btn_new = (
    '                <widget class="QPushButton" name="btnSubEmpl5">\n'
    '                 <property name="text">\n'
    '                  <string>Disponibilité</string>\n'
    '                 </property>\n'
    '                 <property name="checkable">\n'
    '                  <bool>true</bool>\n'
    '                 </property>\n'
    '                 <property name="autoExclusive">\n'
    '                  <bool>true</bool>\n'
    '                 </property>\n'
    '                </widget>\n'
    '               </item>\n'
    '               <item>\n'
    '                <widget class="QPushButton" name="btnSubEmplSmartPort">\n'
    '                 <property name="text">\n'
    '                  <string>Smart Port</string>\n'
    '                 </property>\n'
    '                 <property name="checkable">\n'
    '                  <bool>true</bool>\n'
    '                 </property>\n'
    '                 <property name="autoExclusive">\n'
    '                  <bool>true</bool>\n'
    '                 </property>\n'
    '                </widget>\n'
    '               </item>\n'
    '               <item>\n'
    '                <spacer name="horizontalSpacer">'
)

if "btnSubEmplSmartPort" not in text:
    if btn_anchor not in text:
        raise SystemExit("btn_anchor not found")
    text = text.replace(btn_anchor, btn_new, 1)

page_needle = (
    "               </widget>\n"
    "              </widget>\n"
    "             </widget>\n"
    "            </item>\n"
    "           </layout>\n"
    "          </widget>\n"
    "          <widget class=\"QWidget\" name=\"pagePecheurs\">"
)
page_replacement = (
    "               </widget>\n"
    "              </widget>\n"
    f"{chunk}\n"
    "             </widget>\n"
    "            </item>\n"
    "           </layout>\n"
    "          </widget>\n"
    "          <widget class=\"QWidget\" name=\"pagePecheurs\">"
)

if "subPageEmplSmartPort" not in text:
    if page_needle not in text:
        raise SystemExit("page_needle not found")
    text = text.replace(page_needle, page_replacement, 1)

final.write_text(text, encoding="utf-8")
print("OK", "btnSubEmplSmartPort" in text, "subPageEmplSmartPort" in text)
