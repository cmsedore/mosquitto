#include "config.h"

#include <stdio.h>
#include <string.h>

#include "mosquitto_broker_internal.h"
#include "memory_mosq.h"
#include "mqtt_protocol.h"
#include "packet_mosq.h"
#include "property_mosq.h"

int parse_mount_point(struct mosquitto *context, char **mount_p, char *topic)
{
    char *mount=*mount_p;

    if (strstr(context->listener->mount_point, "%") != NULL) {
        char *s, *m, *u, *i, *o;
        int ln;

        ln = (int) (strlen(topic) + strlen(mount) + 1 + 512);

        mount = mosquitto__realloc(mount, (size_t) ln);
        *mount_p=mount;
        
        if (mount == NULL)
            return MOSQ_ERR_NOMEM;

        s = mount;
        m = context->listener->mount_point;
        u = context->username;
        i = context->id;

        while (*m != 0)
        {
            if (s - mount >= ln)
            {
                return MOSQ_ERR_NOMEM;
            }

            if (*m == '%')
            {
                m++;
                switch (*m)
                {
                case '+':
                    m++;
                    if (*m == 'u')
                    {
                        if (u)
                        {
                            while (*u != '+' && *u != 0)
                            {
                                u++;
                            }
                            if (*u == '+')
                            {
                                u++;
                                while (*u)
                                {
                                    *s = *u;
                                    s++;
                                    u++;
                                }
                                u = context->username;
                            }
                        }
                    }
                    else
                    {
                        m++; // u is the only expected character -- skip whatever came after %+
                    }
                    break;
                case '-':
                    m++;
                    if (*m == 'u')
                    {
                        if (u)
                        {
                            while (*u != '+' && *u != 0)
                            {
                                *s = *u;
                                s++;
                                u++;
                            }
                        }
                        u = context->username;
                    }
                    else
                    {
                        m++; // u is the only expected character -- skip whatever came after %-
                    }
                    break;
                case 'u':
                    m++;
                    if (u)
                    {
                        while (*u)
                        {
                            *s = *u;
                            s++;
                            u++;
                        }
                    }
                    break;
                case 'i':
                    m++;
                    if (i)
                    {
                        while (*i)
                        {
                            *s = *i;
                            s++;
                            i++;
                        }
                    }
                    break;
                }
            }
            *s = *m;
            s++;
            m++;
        }
        if (*(s - 1) == 0)
            s--;
        o = topic;
        while (*o)
        {
            *s = *o;
            s++;
            o++;
        }
        *s = 0;

        return MOSQ_ERR_SUCCESS;
    }

    return MOSQ_ERR_PLUGIN_DEFER;
}