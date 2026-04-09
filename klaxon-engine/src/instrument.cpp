#include "instrument.h"

int Envelope::add_point(int pos)
{
    if (count == MAX_POINTS) return -1;

    // shift everything after pos by 1 and add envelope

    if(pos == count - 1 || pos == -1) {
        points[count].active = true;
        points[count].tick = points[count - 1].tick + 10;
        points[count].vol = points[count - 1].vol;
        count++;
        return 0;
    }
        
    for(int i = count; i > pos + 1; i--)
    {
        points[i] = points[i - 1];
    }

    points[pos + 1].active = true;
    // place between points
    points[pos + 1].tick = static_cast<uint8_t>( floor(
        static_cast<float>((points[pos].tick + points[pos + 2].tick))/2
    ));
    points[pos + 1].vol = points[pos].vol;
    count++;
    return 0;
}

int Envelope::delete_point(int pos)
{
    if (count <= 2) return -1;

    
    if(pos == count - 1 || pos == -1) {
        points[count - 1].active = false;
        count--;
        return 0;
    }
    
    // shift everything backwards

    for(int i = pos; i < count - 1; i++)
    {
        points[i] = points[i + 1];
    }

    points[count - 1].active = false;
    count--;
    return 0;
}