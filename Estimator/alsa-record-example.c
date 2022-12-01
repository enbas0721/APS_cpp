/*
   From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html
   Fixes rate and buffer problems
   sudo apt-get install libasound2-dev
   gcc -o record.out alsa-record-example.c WavManager/audioio.c -lasound
   ./record.out hw:1
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include "WavManager/audioio.h"
#include <time.h>

#include <math.h>

#define SMPL 44100
#define BIT 16

int main (int argc, char *argv[])
{
	// バッファ系の変数
	int i;
	int err;
	int16_t *buffer;
	int buffer_frames = 1024;
	unsigned int rate = SMPL;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

	double recording_time = 10.0f;

	// Wavファイル作成用
	WAV_PRM prm;
	int16_t *record_data;
	char filename[64] = "output.wav";

	// if (argc != 2) {
	// 	printf("Not enough argument(s). This needs 1 arguments.\n");
	// 	exit(1);
	// }

	// for (i = 0; n < sizeof(argv[1]); n++) {
	// 	filename[n] = argv[1][n];
	// }

	// パラメータコピー
	prm.fs = SMPL;
	prm.bits = BIT;
	prm.L =  prm.fs * recording_time;

	if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stdout, "cannot open audio device %s (%s)\n",
		         argv[1],
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "audio interface opened\n");

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stdout, "cannot allocate hardware parameter structure (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params allocated\n");

	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stdout, "cannot initialize hardware parameter structure (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params initialized\n");

	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stdout, "cannot set access type (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params access setted\n");

	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
		fprintf (stdout, "cannot set sample format (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params format setted\n");

	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stdout, "cannot set sample rate (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params rate setted\n");

	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
		fprintf (stdout, "cannot set channel count (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params channels setted\n");

	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stdout, "cannot set parameters (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params setted\n");

	snd_pcm_hw_params_free (hw_params);

	fprintf(stdout, "hw_params freed\n");

	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stdout, "cannot prepare audio interface for use (%s)\n",
		         snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "audio interface prepared\n");

	// record_data = calloc(prm.L, sizeof(double));
	// buffer = (double*)malloc(sizeof(double)*buffer_frames*snd_pcm_format_width(format));
	record_data = calloc(prm.L, sizeof(double));
	buffer = (int16_t*)malloc(sizeof(int16_t)*buffer_frames*snd_pcm_format_width(format));

	fprintf(stdout, "buffer allocated\n");

	// time_t start_time = time(NULL);
	// time_t elapsed_time = time(NULL) - start_time;
	int current_index = 0;
	while ((current_index + buffer_frames) < prm.L) {
		if ((err = snd_pcm_readi(capture_handle, (void*)buffer, buffer_frames)) != buffer_frames) {
			fprintf(stdout, "read from audio interface failed (%s)\n",err, snd_strerror(err));
			exit (1);
		}
		fprintf(stdout, "Read buffer first 5: ");
		for(int j = 0; j < 5; j++) {
			fprintf(stdout, "%d ", buffer[j]);
		}
		printf("\n");
		for (int i = current_index; i < current_index + err; i++) {
			record_data[i] = buffer[i-current_index];
		}
		fprintf(stdout, "Record data first 5: ");
		for(int j = current_index; j < current_index + 5; j++) {
			fprintf(stdout, "%d ", record_data[j]);
		}
		printf("\n");
		current_index = current_index + err;
		// elapsed_time = time(NULL) - start_time;
	}

	audio_write(record_data, &prm, filename);

	free(buffer);
	free(record_data);

	fprintf(stdout, "buffer freed\n");
	snd_pcm_close (capture_handle);
	fprintf(stdout, "audio interface closed\n");

	exit (0);
	return 0;
}
