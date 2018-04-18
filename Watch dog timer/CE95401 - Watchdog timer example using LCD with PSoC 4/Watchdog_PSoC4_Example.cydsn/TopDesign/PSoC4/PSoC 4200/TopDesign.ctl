-- =============================================================================
-- The following directives assign pins to the locations specific for the
-- CY8CKIT-042 kit.
-- =============================================================================

-- === RGB LED ===
attribute port_location of LED_Reset(0) : label is "PORT(1,6)"; -- RED LED
attribute port_location of LED_WdtInt(0) : label is "PORT(0,2)"; -- GREEN LED
attribute port_location of LED_WdtReset(0) : label is "PORT(0,3)"; -- BLUE LED
