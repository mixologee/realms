
void nmudlog(int type, int level, byte file, const char *targ, ...);


// NMUDLOG Types
#define INFO_LOG     0   //General Information
#define MISC_LOG     1   //Misc Information
#define SYS_LOG      2   //System Information
#define OLC_LOG      3   //OLC Information
#define GOD_LOG      4   //GOD Commands Information
#define DEATH_LOG    5   //Death Information
#define ZONE_LOG     6   //Zone Reset Information
#define CONN_LOG     7   //Connection Information
#define SUGG_LOG     8   //Suggestions
#define BUG_LOG      9   //Bugs
#define IDEA_LOG    10   //Ideas
#define TYPO_LOG    11   //Typos
#define GIVE_LOG    12   //Anything Given
#define TAKE_LOG    13   //God Take Log

#define MAX_LOG     14   //Set +1 everytime a new one is added


// PLAYER LOG FLAGS
#define LOG_INFO        (1 << 0)    
#define LOG_MISC        (1 << 1)
#define LOG_SYS         (1 << 2)
#define LOG_OLC         (1 << 3)
#define LOG_GOD         (1 << 4)
#define LOG_DEATH       (1 << 5)
#define LOG_ZONE        (1 << 6)
#define LOG_CONN        (1 << 7)
#define LOG_SUGG        (1 << 8)
#define LOG_BUG         (1 << 9)
#define LOG_IDEA        (1 << 10)
#define LOG_TYPO        (1 << 11)
#define LOG_GIVE        (1 << 12)
#define LOG_TAKE        (1 << 13)
#define MAX_LOG_FLAG    (1 << 14)
