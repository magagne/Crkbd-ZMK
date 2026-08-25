if(CONFIG_RAW_HID)

    target_sources(app PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../config/src/behavior_raw_drag_scroll.c
    )

endif()