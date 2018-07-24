#ifndef __DRV_ADC_H_INCLUDE__
#define __DRV_ADC_H_INCLUDE__

struct rt_adc_ops
{
    rt_err_t (*convert)(struct rt_device *device, int channel, rt_uint16_t *value);
};

struct rt_device_adc
{
    struct rt_device parent;
    const struct rt_adc_ops *ops;
};

extern rt_err_t rt_device_adc_register(struct rt_device_adc *device, const char *name, const struct rt_adc_ops *ops, const void *user_data);

extern rt_err_t rt_device_adc_create(const char *name, const struct rt_adc_ops *ops, const void *user_data);

#endif /* __DRV_ADC_H_INCLUDE__ */

