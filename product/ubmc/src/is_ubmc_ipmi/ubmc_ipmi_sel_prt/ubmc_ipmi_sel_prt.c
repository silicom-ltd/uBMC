#include <stdio.h>
#include "ubmc_ipmi.h"
#include "ubmc_ipmi_sel.h"
int main()
{
	struct ubmc_ipmi_sel ubmc_ipmi_sel;
	struct sel_event_record event;
	int ret = 0;
	ubmc_ipmi_sel.sel_msg_fd = create_sel_file_on_emmc("/var/log/sel");
	if(ubmc_ipmi_sel.sel_msg_fd == NULL)
	{
		UBMC_IPMI_DEBUG_ERR("OPEN /var/log/sel FAIL");
		return 1;
	}
    ret = ubmc_ipmi_prt_sel(&ubmc_ipmi_sel);
    return 0;

}
