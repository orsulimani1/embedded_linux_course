#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork(), read(), write(), close()
#include <string.h>     // strlen()
#include <fcntl.h>      // open(), O_RDONLY, O_WRONLY, O_NONBLOCK
#include <errno.h>      // errno , EINTR
#define FILENAME "../../../lesson_3/code_examples/who_lets_the_dogs_out"

// ssize_t write(int fd, const void *buf, size_t count);
char * song = "Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Well, the party was nice, the party was pumpin\'\n\
Heya, yippie yi yo\n\
And everybody havin\' a ball\n\
Huh, huh, yippie yi yo\n\
I tell the fellas start the name callin\'\n\
Yippie yi yo\n\
And the girls respond to the call\n\
I heard a woman shout out\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
I see de dance people had a ball\n\
\'Cause she really want to skip town\n\
Get back, Gruffy, back, Scruffy\n\
Get back you flea infested mongre\n\
Gonna tell myself, \"hey, man no get angry\"\n\
Heya, yippie yi yo\n\
To any girls callin\' them canine\n\
Hey, yippie yi yo\n\
But they tell me, \"hey, man, it\'s part of the party\"\n\
Yippie yi yo\n\
You put a woman in front and her man behind\n\
I heard a woman shout out\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Say, a doggy is nuttin\' if he don' have a bone\n\
All doggy, hold ya\' bone, all doggy, hold it\n\
A doggy is nuttin\' if he don' have a bone\n\
All doggy, hold ya\' bone, all doggy, hold it\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
I see de dance people had a ball\n\
'Cause she really want to skip town\n\
Get back, Gruffy, back, Scruffy\n\
Get back you flea infested mongrel\n\
Well, if I am a dog, the party is on\n\
I gotta get my groove 'cause my mind done gone\n\
Do you see the rays comin' from my eye\n\
Walkin' through the place that digi-man is breakin' it down?\n\
Me and my white short shorts\n\
And I can't see color, any color will do\n\
I'll stick on you, that's why they call me \"pit bull\"\n\
'Cause I'm the man of the land\n\
When they see me they say, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Well, the party was nice, the party was pumpin\'\n\
Heya, yippie yi yo\n\
And everybody havin\' a ball\n\
Huh, huh, yippie yi yo\n\
I tell the fellas start the name callin\'\n\
Yippie yi yo\n\
And the girls respond to the call\n\
I heard a woman shout out\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
I see de dance people had a ball\n\
\'Cause she really want to skip town\n\
Get back, Gruffy, back, Scruffy\n\
Get back you flea infested mongre\n\
Gonna tell myself, \"hey, man no get angry\"\n\
Heya, yippie yi yo\n\
To any girls callin\' them canine\n\
Hey, yippie yi yo\n\
But they tell me, \"hey, man, it\'s part of the party\"\n\
Yippie yi yo\n\
You put a woman in front and her man behind\n\
I heard a woman shout out\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Say, a doggy is nuttin\' if he don' have a bone\n\
All doggy, hold ya\' bone, all doggy, hold it\n\
A doggy is nuttin\' if he don' have a bone\n\
All doggy, hold ya\' bone, all doggy, hold it\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
I see de dance people had a ball\n\
'Cause she really want to skip town\n\
Get back, Gruffy, back, Scruffy\n\
Get back you flea infested mongrel\n\
Well, if I am a dog, the party is on\n\
I gotta get my groove 'cause my mind done gone\n\
Do you see the rays comin' from my eye\n\
Walkin' through the place that digi-man is breakin' it down?\n\
Me and my white short shorts\n\
And I can't see color, any color will do\n\
I'll stick on you, that's why they call me \"pit bull\"\n\
'Cause I'm the man of the land\n\
When they see me they say, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who?\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)\n\
Who let the dogs out?\n\
Who, who, who, who, who? (Yippie yi yo)";

int main(int argc, char const *argv[])
{   
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC , 0644);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    /* code */
    ssize_t ret, len = strlen(song);
    size_t total_written = 0;

    while (len != 0 && (ret = write (fd, song, len)) != 0) {
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            perror ("write");
            break;
            }
        // Print diagnostic info (optional)
        printf("Wrote %zd bytes\n",ret);
        len -= ret;
        song += ret;
        sleep(1);
    }

    return 0;
}


