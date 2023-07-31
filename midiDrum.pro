QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

RC_ICONS = drumMidiIcon.ico


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
 MidiRules/Rules/activatetabrule.cpp \
 MidiRules/Rules/createmessagerule.cpp \
 MidiRules/Rules/dropallrule.cpp \
 tab.cpp \
 workerthread.cpp \
 MidiRules/ruletypes.cpp \
 MidiRules/Rules/dropnoterule.cpp \
 MidiRules/Rules/remaprule.cpp \
 MidiRules/Rules/velocityrule.cpp \
 MidiRules/Rules/waitfornoterule.cpp \
 MidiRules/Rules/waitfortimerule.cpp \
 MidiRules/rulescontainer.cpp \
 RtMidi.cpp \
 ScrollBar/scrollbar.cpp \
 ScrollBar/scrollbarhandle.cpp \
 ScrollBar/scrollbarparams.cpp \
 erroroverlay.cpp \
 main.cpp \
 mainwindow.cpp \
 midimonitor.cpp \
 midinote.cpp \
 midinotescontainer.cpp \
 titlebar.cpp


HEADERS += \
 MidiRules/Rules/activatetabrule.h \
 MidiRules/Rules/createmessagerule.h \
 MidiRules/Rules/dropallrule.h \
 tab.h \
 workerthread.h \
 MidiRules/rule.h \
 MidiRules/ruletypes.h \
 MidiRules/Rules/dropnoterule.h \
 MidiRules/Rules/remaprule.h \
 MidiRules/Rules/velocityrule.h \
 MidiRules/Rules/waitfornoterule.h \
 MidiRules/Rules/waitfortimerule.h \
 MidiRules/rulescontainer.h \
 RtMidi.h \
 ScrollBar/scrollbar.h \
 ScrollBar/scrollbarhandle.h \
 ScrollBar/scrollbarparams.h \
 erroroverlay.h \
 globals.h \
 mainwindow.h \
 midimonitor.h \
 midinote.h \
 midinotescontainer.h \
 titlebar.h



FORMS += \
    mainwindow.ui




# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Images.qrc


win32: LIBS += -lodbc32
win32: LIBS += -lodbccp32
win32: LIBS += -lWinMM


win32: LIBS += -L$$PWD/'../../../../../../Program Files (x86)/Tobias Erichsen/teVirtualMIDISDK/C-Binding/' -lteVirtualMIDI64
INCLUDEPATH += $$PWD/'../../../../../../Program Files (x86)/Tobias Erichsen/teVirtualMIDISDK/C-Binding'
DEPENDPATH += $$PWD/'../../../../../../Program Files (x86)/Tobias Erichsen/teVirtualMIDISDK/C-Binding'
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/'../../../../../../Program Files (x86)/Tobias Erichsen/teVirtualMIDISDK/C-Binding/teVirtualMIDI64.lib'


# for GetScaleFactorForMonitor
win32: LIBS += -L"C:/Users/StRaToX/Documents/Qt Projects/drumMidi_App/thirdParty/UIScalingForMonitorThisAppIsOn/" -lDll_UIScalingForMonitorTheAppIsOn
INCLUDEPATH += 'C:/Users/StRaToX/Documents/Qt Projects/drumMidi_App/thirdParty/UIScalingForMonitorThisAppIsOn/include'
