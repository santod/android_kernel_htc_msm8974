#include <linux/platform_device.h>
#include <mach/devices_cmdline.h>
#include <mach/devices_dtb.h>
#include <linux/module.h>

#define RESET_MSG_LENGTH 512
static unsigned char google_boot_reason[RESET_MSG_LENGTH];
int __init google_boot_reason_init(char *s)
{
	snprintf(google_boot_reason, sizeof(google_boot_reason) - 1,
		"Boot info:\nLast boot reason: %s\n\n", s);

        return 1;
}
__setup("bootreason=", google_boot_reason_init);

unsigned char *board_get_google_boot_reason(void)
{
        return google_boot_reason;
}
EXPORT_SYMBOL(board_get_google_boot_reason);

#if defined(CONFIG_LCD_KCAL)
#include <mach/htc_lcd_kcal.h>
#endif

static unsigned long boot_powerkey_debounce_ms;
int __init boot_powerkey_debounce_time_init(char *s)
{
        int ret;
        ret = strict_strtoul(s, 16, &boot_powerkey_debounce_ms);
        if (ret != 0)
                pr_err("%s: boot_powerkey_debounce_ms cannot be parsed from `%s'\r\n", __func__, s);
        return 1;
}
__setup("bpht=", boot_powerkey_debounce_time_init);

int board_get_boot_powerkey_debounce_time(void)
{
        return boot_powerkey_debounce_ms;
}
EXPORT_SYMBOL(board_get_boot_powerkey_debounce_time);

static unsigned long usb_ats;
int __init board_ats_init(char *s)
{
	int ret;
	ret = strict_strtoul(s, 16, &usb_ats);
	if (ret != 0)
		pr_err("%s: usb_ats cannot be parsed from `%s'\r\n", __func__, s);
	return 1;
}
__setup("ats=", board_ats_init);


int board_get_usb_ats(void)
{
	if (get_debug_flag() & DEBUG_FLAG_ENABLE_ATS_FLAG)
		return 1;
	else
		return usb_ats;
}
EXPORT_SYMBOL(board_get_usb_ats);

void board_set_usb_ats(int type)
{
	if (type == 0)
		usb_ats = 0;
	else
		usb_ats = 1;
}
EXPORT_SYMBOL(board_set_usb_ats);

#define RAW_SN_LEN	4
static int tamper_sf;
static char android_serialno[16] = {0};
static int __init board_serialno_setup(char *serialno)
{
	if (tamper_sf) {
		int i;
		char hashed_serialno[16] = {0};
		strncpy(hashed_serialno, serialno, sizeof(hashed_serialno)/sizeof(hashed_serialno[0]) - 1);
		for (i = strlen(hashed_serialno) - 1; i >= RAW_SN_LEN; i--)
			hashed_serialno[i - RAW_SN_LEN] = '*';
		pr_info("%s: set serial no to %s\r\n", __func__, hashed_serialno);
	} else {
		pr_info("%s: set serial no to %s\r\n", __func__, serialno);
	}
	strncpy(android_serialno, serialno, sizeof(android_serialno)/sizeof(android_serialno[0]) - 1);
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

char *board_serialno(void)
{
	return android_serialno;
}
EXPORT_SYMBOL(board_serialno);

static char android_mid[64] = {0};
static int __init board_mid_setup(char *mid)
{
	pr_info("%s: set mid to %s\r\n", __func__, mid);
	strncpy(android_mid, mid, sizeof(android_mid)/sizeof(android_mid[0]) - 1);
	return 1;
}
__setup("androidboot.mid=", board_mid_setup);

char *board_mid(void)
{
	return android_mid;
}
EXPORT_SYMBOL(board_mid);

static int mfg_mode;
static int fullramdump_flag;
static int recovery_9k_ramdump;
int __init board_mfg_mode_init(char *s)
{
	if (!strcmp(s, "normal"))
		mfg_mode = MFG_MODE_NORMAL ;
	else if (!strcmp(s, "factory2"))
		mfg_mode = MFG_MODE_FACTORY2;
	else if (!strcmp(s, "recovery"))
		mfg_mode = MFG_MODE_RECOVERY;
	else if (!strcmp(s, "charge"))
		mfg_mode = MFG_MODE_CHARGE;
	else if (!strcmp(s, "power_test"))
		mfg_mode = MFG_MODE_POWER_TEST;
	else if (!strcmp(s, "offmode_charging"))
		mfg_mode = MFG_MODE_OFFMODE_CHARGING;
	else if (!strcmp(s, "mfgkernel:diag58"))
		mfg_mode = MFG_MODE_MFGKERNEL_DIAG58;
	else if (!strcmp(s, "gift_mode"))
		mfg_mode = MFG_MODE_GIFT_MODE;
	else if (!strcmp(s, "mfgkernel"))
		mfg_mode = MFG_MODE_MFGKERNEL;
	else if (!strcmp(s, "mini") || !strcmp(s, "skip_9k_mini")) {
		mfg_mode = MFG_MODE_MINI;
		fullramdump_flag = 0;
	} else if (!strcmp(s, "mini:1gdump")) {
		mfg_mode = MFG_MODE_MINI;
		fullramdump_flag = 1;
	}

	if (!strncmp(s, "9kramdump", strlen("9kramdump")))
		recovery_9k_ramdump = 1 ;

	return 1;
}
__setup("androidboot.mode=", board_mfg_mode_init);


int board_mfg_mode(void)
{
	return mfg_mode;
}

EXPORT_SYMBOL(board_mfg_mode);

int is_9kramdump_mode(void)
{
	return recovery_9k_ramdump;
}

int board_fullramdump_flag(void)
{
	return fullramdump_flag;
}

EXPORT_SYMBOL(board_fullramdump_flag);

static int build_flag;
static int __init board_bootloader_setup(char *str)
{
	char temp[strlen(str) + 1];
	char *p = NULL;
	char *build = NULL;
	char *args = temp;

	printk(KERN_INFO "%s: %s\n", __func__, str);

	strcpy(temp, str);

	
	while ((p = strsep(&args, ".")) != NULL) build = p;

	if (build) {
		if (build[0] == '0') {
			build_flag = BUILD_MODE_SHIP;
		} else if (build[0] == '2') {
			build_flag = BUILD_MODE_ENG;
		} else if (build[0] == '1') {
			build_flag = BUILD_MODE_MFG;
		} else {
			build_flag = BUILD_MODE_ENG;
		}
		pr_info("%s: set build flag to %d\n", __func__, build_flag);
	}
	return 1;
}
__setup("androidboot.bootloader=", board_bootloader_setup);

#if defined(CONFIG_LCD_KCAL)
int g_kcal_r = 255;
int g_kcal_g = 255;
int g_kcal_b = 255;

extern int kcal_set_values(int kcal_r, int kcal_g, int kcal_b);
static int __init display_kcal_setup(char *kcal)
{
	char vaild_k = 0;
	int kcal_r = 255;
	int kcal_g = 255;
	int kcal_b = 255;

	sscanf(kcal, "%d|%d|%d|%c", &kcal_r, &kcal_g, &kcal_b, &vaild_k );
	pr_info("kcal is %d|%d|%d|%c\n", kcal_r, kcal_g, kcal_b, vaild_k);

	if (vaild_k != 'K') {
		pr_info("kcal not calibrated yet : %d\n", vaild_k);
		kcal_r = kcal_g = kcal_b = 255;
	}

	kcal_set_values(kcal_r, kcal_g, kcal_b);
	return 1;
}
__setup("htc.kcal=", display_kcal_setup);
#endif

int board_build_flag(void)
{
	return build_flag;
}
EXPORT_SYMBOL(board_build_flag);

BLOCKING_NOTIFIER_HEAD(psensor_notifier_list);
int register_notifier_by_psensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&psensor_notifier_list, nb);
}

int unregister_notifier_by_psensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&psensor_notifier_list, nb);
}

BLOCKING_NOTIFIER_HEAD(hallsensor_notifier_list);
int register_notifier_by_hallsensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&hallsensor_notifier_list, nb);
}
EXPORT_SYMBOL(register_notifier_by_hallsensor);

int unregister_notifier_by_hallsensor(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&hallsensor_notifier_list, nb);
}
EXPORT_SYMBOL(unregister_notifier_by_hallsensor);

int __init check_tamper_sf(char *s)
{
	tamper_sf = simple_strtoul(s, 0, 10);
	return 1;
}
__setup("td.sf=", check_tamper_sf);

unsigned int get_tamper_sf(void)
{
	return tamper_sf;
}
EXPORT_SYMBOL(get_tamper_sf);
