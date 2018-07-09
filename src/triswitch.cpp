#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <lv2.h>

/**********************************************************************************************************************************************************/

#define PLUGIN_URI "http://Plujain/plugins/triswitch"
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define RAIL(v, min, max) (MIN((max), MAX((min), (v))))
#define RAILZO(v) (RAIL(v, 0.0f, 1.0f))

enum {IN, OUT_A, OUT_B, OUT_C, CHANNEL, CROSS_LENGTH_A, SHAPE_A, CROSS_LENGTH_B, SHAPE_B, CROSS_LENGTH_C, SHAPE_C, PLUGIN_PORT_COUNT};

/**********************************************************************************************************************************************************/

struct Out { float gain; float *cross_length; float *shape; float *out; }; 

class TriSwitch
{
public:
    TriSwitch() {}
    ~TriSwitch() {}
    static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features);
    static void activate(LV2_Handle instance);
    static void deactivate(LV2_Handle instance);
    static void connect_port(LV2_Handle instance, uint32_t port, void *data);
    static void run(LV2_Handle instance, uint32_t n_samples);
    static void cleanup(LV2_Handle instance);
    static const void* extension_data(const char* uri);
    
    Out outs[3];
    int number_of_outs;
    
    float *in;
    float *asked_channel;
    float fade_pos;
    int ex_channel;
    double samplerate;
};

/**********************************************************************************************************************************************************/

static const LV2_Descriptor Descriptor = {
    PLUGIN_URI,
    TriSwitch::instantiate,
    TriSwitch::connect_port,
    TriSwitch::activate,
    TriSwitch::run,
    TriSwitch::deactivate,
    TriSwitch::cleanup,
    TriSwitch::extension_data
};

/**********************************************************************************************************************************************************/

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index == 0) return &Descriptor;
    else return NULL;
}

/**********************************************************************************************************************************************************/

LV2_Handle TriSwitch::instantiate(const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
    TriSwitch *plugin = new TriSwitch();
    
    plugin->number_of_outs = 3;
    
    for (int i=0; i < plugin->number_of_outs ; i++){
        if (i == 0){
            plugin->outs[i].gain = 1.0f;
        } else {
            plugin->outs[i].gain = 0.0f;
        }
            
    }
    plugin->fade_pos = 0.00f;
    plugin->ex_channel = 0;
    plugin->samplerate = samplerate;
    
    return (LV2_Handle)plugin;
}

/**********************************************************************************************************************************************************/

void TriSwitch::activate(LV2_Handle instance)
{
    // TODO: include the activate function code here
}

/**********************************************************************************************************************************************************/

void TriSwitch::deactivate(LV2_Handle instance)
{
    // TODO: include the deactivate function code here
}
        

/**********************************************************************************************************************************************************/

void TriSwitch::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    TriSwitch *plugin;
    plugin = (TriSwitch *) instance;

    switch (port)
    {
        case IN:
            plugin->in = (float*) data;
            break;
        case OUT_A:
            plugin->outs[0].out = (float*) data;
            break;
        case OUT_B:
            plugin->outs[1].out = (float*) data;
            break;
        case OUT_C:
            plugin->outs[2].out = (float*) data;
            break;
        case CHANNEL:
            plugin->asked_channel = (float*) data;
            break;
        case CROSS_LENGTH_A:
            plugin->outs[0].cross_length = (float*) data;
            break;
        case SHAPE_A:
            plugin->outs[0].shape = (float*) data;
            break;
        case CROSS_LENGTH_B:
            plugin->outs[1].cross_length = (float*) data;
            break;
        case SHAPE_B:
            plugin->outs[1].shape = (float*) data;
            break;
        case CROSS_LENGTH_C:
            plugin->outs[2].cross_length = (float*) data;
            break;
        case SHAPE_C:
            plugin->outs[2].shape = (float*) data;
            break;
    }
    
}

/**********************************************************************************************************************************************************/

void TriSwitch::run(LV2_Handle instance, uint32_t n_samples)
{
    TriSwitch *plugin;
    plugin = (TriSwitch *) instance;
    
    int canal;
    canal = (int)(*(plugin->asked_channel));
    
    Out *output;
    output = &plugin->outs[canal];
    
    if (canal != plugin->ex_channel){
        plugin->fade_pos = 0.0f;
    }

    for ( uint32_t i = 0; i < n_samples; i++)
    {
        if (plugin->fade_pos >= 1.00f){
            output->out[i] = plugin->in[i];
            
            for (int j = 0; j < plugin->number_of_outs; j++)
            {
                if (j != canal){ 
                    plugin->outs[j].out[i] = 0.0f;
                }
            }
            continue;
        }
        
        
        if (*output->cross_length > 0.00f){
            plugin->fade_pos += 1/((*output->cross_length) * plugin->samplerate);
            plugin->fade_pos = RAILZO(plugin->fade_pos);
        } else {
            plugin->fade_pos = 1.00f;
        }
        
        float gain;
        
        if (*output->shape < 0.0f){
            gain = RAILZO(plugin->fade_pos - (sin(M_PI_2 * (plugin->fade_pos -1)) +1 - plugin->fade_pos) * *output->shape);
        } else {
            gain = RAILZO(plugin->fade_pos + (sin(M_PI_2 * plugin->fade_pos) - plugin->fade_pos) * *output->shape);
        }
        
        if (gain <= output->gain){
            for (int j = 0; j < plugin->number_of_outs; j++)
            {
                plugin->outs[j].out[i] = plugin->in[i] * plugin->outs[j].gain;
            }
            continue;
        }
        
        float delta;
        delta = (float) (gain - output->gain);
        
        output->gain = gain;
        output->out[i] = plugin->in[i] * output->gain;
        
        for (int j = 0; j < plugin->number_of_outs; j++)
        {
            if (j != canal){
                float ratio = plugin->outs[j].gain / (1.0f - output->gain);
                plugin->outs[j].gain = RAILZO(plugin->outs[j].gain - (delta * ratio));
                plugin->outs[j].out[i] = plugin->in[i] * plugin->outs[j].gain;
            }
        }
    }
    plugin->ex_channel = canal;
}

/**********************************************************************************************************************************************************/

void TriSwitch::cleanup(LV2_Handle instance)
{
    delete ((TriSwitch *) instance);
}

/**********************************************************************************************************************************************************/

const void* TriSwitch::extension_data(const char* uri)
{
    return NULL;
}
