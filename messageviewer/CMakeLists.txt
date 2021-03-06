set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(tests)

set(libmessageviewer_SRCS

viewer_p.cpp
viewer.cpp
khtmlparthtmlwriter.cpp
htmlstatusbar.cpp
headerstyle.cpp
headerstrategy.cpp
kmaddrbook.cpp
stringutil.cpp
attachmentstrategy.cpp
kmmsgpartdlg.cpp
csshelper.cpp
util.cpp
objecttreeparser_p.cpp
objecttreeparser.cpp
kleojobexecutor.cpp
bodypartformatter.cpp
bodypartformatterfactory.cpp
partnodebodypart.cpp
nodehelper.cpp
urlhandlermanager.cpp
vcardviewer.cpp
iconnamecache.cpp
mailsourceviewer.cpp
mimetreemodel.cpp
global.cpp
spamheaderanalyzer.cpp
antispamconfig.cpp
globalsettings.cpp
configurewidget.cpp
htmlquotecolorer.cpp
editorwatcher.cpp
attachmentdialog.cpp
objecttreeviewersource.cpp
objecttreeemptysource.cpp
filehtmlwriter.cpp
teehtmlwriter.cpp
)


kde4_add_kcfg_files(libmessageviewer_SRCS
  globalsettings_base.kcfgc
)

kde4_add_ui_files(libmessageviewer_SRCS
  settings.ui
)

kde4_add_library(messageviewer SHARED ${libmessageviewer_SRCS})


target_link_libraries(messageviewer messagecore kdepim kpgp kleo
 ${KDEPIMLIBS_KMIME_LIBS}
 ${KDEPIMLIBS_KPIMUTILS_LIBS} ${KDEPIMLIBS_KABC_LIBS}
 ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KTNEF_LIBRARY}
 ${QGPGME_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/libkleo
    ${Boost_INCLUDE_DIRS}
    ${GPGME_INCLUDES}
)

set_target_properties( messageviewer PROPERTIES VERSION 0.1.0 SOVERSION 0)
install(TARGETS messageviewer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer.h attachmentstrategy.h headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)
