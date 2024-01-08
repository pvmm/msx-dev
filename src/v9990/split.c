#include <stdint.h>
#include <string.h>

#include "v99xx.h"
#include "g9k.h"
#include "fusion-c.h"


int init_video()
{
	// V9990 VDP initialization
	if (init_g9k() != OK) {
		return INITG9_ERROR;
	}

	// using superimpose if Video9000
	set_ctrl_port(SUPERIMPOSE_TRANSPARENT);

	// init sprite pattern table at 0x10000
	init_sprite_pattern(4);
	init_pattern_mode();

	if (load_assets() != OK) {
		return LOADASSETS_ERROR;
	}
}


int main()
{
	disable_click();

	if (init_platform() < 0) {
		return -1;
	}

	enable_vblank_hook();
	wait_vsync();

	while (true) {
		idle_update();
	}

	return 0;
}
