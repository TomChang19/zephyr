/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bme280.h"
#include <math.h>

static int bme280_decoder_get_frame_count(const uint8_t *buffer, struct sensor_chan_spec chan_spec,
					  uint16_t *frame_count)
{
	const struct bme280_encoded_data *edata = (const struct bme280_encoded_data *)buffer;
	int32_t ret = -ENOTSUP;

	if (chan_spec.chan_idx != 0) {
		return ret;
	}

	/* This sensor lacks a FIFO; there will always only be one frame at a time. */
	switch (chan_spec.chan_type) {
	case SENSOR_CHAN_AMBIENT_TEMP:
		*frame_count = edata->has_temp ? 1 : 0;
		break;
	case SENSOR_CHAN_PRESS:
		*frame_count = edata->has_press ? 1 : 0;
		break;
	case SENSOR_CHAN_HUMIDITY:
		*frame_count = edata->has_humidity ? 1 : 0;
		break;
	default:
		return ret;
	}

	if (*frame_count > 0) {
		ret = 0;
	}

	return ret;
}

static int bme280_decoder_get_size_info(struct sensor_chan_spec chan_spec, size_t *base_size,
					size_t *frame_size)
{
	switch (chan_spec.chan_type) {
	case SENSOR_CHAN_AMBIENT_TEMP:
	case SENSOR_CHAN_HUMIDITY:
	case SENSOR_CHAN_PRESS:
		*base_size = sizeof(struct sensor_q31_sample_data);
		*frame_size = sizeof(struct sensor_q31_sample_data);
		return 0;
	default:
		return -ENOTSUP;
	}
}

static int bme280_decoder_decode(const uint8_t *buffer, struct sensor_chan_spec chan_spec,
				 uint32_t *fit, uint16_t max_count, void *data_out)
{
	const struct bme280_encoded_data *edata = (const struct bme280_encoded_data *)buffer;

	if (*fit != 0) {
		return 0;
	}

	struct sensor_q31_data *out = data_out;

	out->header.base_timestamp_ns = edata->header.timestamp;
	out->header.reading_count = 1;

	switch (chan_spec.chan_type) {
	case SENSOR_CHAN_AMBIENT_TEMP:
		if (edata->has_temp) {
			int32_t readq = edata->reading.comp_temp * pow(2, 31 - BME280_TEMP_SHIFT);
			int32_t convq = BME280_TEMP_CONV * pow(2, 31 - BME280_TEMP_SHIFT);

			out->readings[0].temperature =
				(int32_t)((((int64_t)readq) << (31 - BME280_TEMP_SHIFT)) /
					  ((int64_t)convq));
			out->shift = BME280_TEMP_SHIFT;
		} else {
			return -ENODATA;
		}
		break;
	case SENSOR_CHAN_PRESS:
		if (edata->has_press) {
			int32_t readq = edata->reading.comp_press;
			int32_t convq = BME280_PRESS_CONV_KPA * pow(2, 31 - BME280_PRESS_SHIFT);

			out->readings[0].pressure =
				(int32_t)((((int64_t)readq) << (31 - BME280_PRESS_SHIFT)) /
					  ((int64_t)convq));
			out->shift = BME280_PRESS_SHIFT;
		} else {
			return -ENODATA;
		}
		break;
	case SENSOR_CHAN_HUMIDITY:
		if (edata->has_humidity) {
			out->readings[0].humidity = edata->reading.comp_humidity;
			out->shift = BME280_HUM_SHIFT;
		} else {
			return -ENODATA;
		}
		break;
	default:
		return -EINVAL;
	}

	*fit = 1;

	return 1;
}

SENSOR_DECODER_API_DT_DEFINE() = {
	.get_frame_count = bme280_decoder_get_frame_count,
	.get_size_info = bme280_decoder_get_size_info,
	.decode = bme280_decoder_decode,
};

int bme280_get_decoder(const struct device *dev, const struct sensor_decoder_api **decoder)
{
	ARG_UNUSED(dev);
	*decoder = &SENSOR_DECODER_NAME();

	return 0;
}
