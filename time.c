#include <stdio.h>
#include <time.h>

int main() {
    time_t timer;
    // This function sets the current time from local in timer
    time(&timer);

    printf("Your local time is: %s", ctime(&timer));
    return 0;
}
