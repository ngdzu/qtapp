set(Z_MONITOR_SOURCES
    main.cpp
)

add_executable(z-monitor
    ${Z_MONITOR_SOURCES}
)

target_link_libraries(z-monitor
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)


