/*
 * Transmitter.cpp
 *
 *  Created on: Nov 17, 2014
 */

#include "Transmitter.h"
#include "DigitizerSimLogger.h"

Transmitter::Transmitter() {
	centerFrequency = -1;
	rdsText = "REDHAWK Radio";
	numSamples = -1;
	initilized = false;

	fm_mpx_status_struct.phase_38 = 0;
	fm_mpx_status_struct.phase_19 = 0;
	fm_mpx_status_struct.audio_index = 0;
	fm_mpx_status_struct.audio_len = 0;
	fm_mpx_status_struct.fir_index = 0;


	unsigned int i;
	for (i = 0; i < FIR_SIZE; i++) {
		this->fm_mpx_status_struct.fir_buffer_mono[i] = 0;
		this->fm_mpx_status_struct.fir_buffer_stereo[i] = 0;
	}

}

Transmitter::~Transmitter() {
}


void Transmitter::setCenterFrequency(float centerFreqeuncy) {
	TRACE("Entered Method");
	TRACE("Setting Center Frequency to : " << centerFreqeuncy);
	this->centerFrequency = centerFreqeuncy;
	TRACE("Exited Method");
}

void Transmitter::setFilePath(path filePath) {
	TRACE("Entered Method");
	TRACE("Setting File Path to : " << filePath.string());
	this->filePath = filePath;
	TRACE("Exited Method");
}

path Transmitter::getFilePath() {
	TRACE("Entered Method");
	return this->filePath;
	TRACE("Exited Method");
}

void Transmitter::setRdsText(std::string rdsText) {
	TRACE("Entered Method");
	TRACE("Setting RDS Text to: " << rdsText);
	this->rdsText = rdsText;
	TRACE("Exited Method");
}

/**
 * TODO: This does not seem ideal.  The creation of a thread is time consuming and resource heavy.  We should have the thread in a wait state
 * and then on start it takes off.
 */
void Transmitter::start(int numSamples) {
	TRACE("Entered Method");

	if (not initilized) {
		ERROR("Transmitter asked to start but has not been initialized!  Request ignored.");
	} else {
		TRACE("Starting boost thread");
		m_Thread = boost::thread(&Transmitter::doWork, this);
	}

	TRACE("Exited Method");
}


void Transmitter::join() {
	TRACE("Entered Method");
	TRACE("joining thread");
	this->m_Thread.join();
	TRACE("Exited Method");
}

int Transmitter::init(int numSamples) {
	TRACE("Entered Method");
	this->numSamples = numSamples;

	TRACE("Initializing RDS struct");
	rds_status_struct.pi = 0x1234;
	set_rds_ps(const_cast<char *> (rdsText.c_str()), &rds_status_struct);
	set_rds_rt(const_cast<char *> (rdsText.c_str()), &rds_status_struct);


	TRACE("Opening fm mpx file");
    if(fm_mpx_open(const_cast<char *> (filePath.string().c_str()), numSamples, &fm_mpx_status_struct) != 0) {
        ERROR("Could not setup FM mulitplex generator.");
        return -1;
    }

    TRACE("Clearing MPX vector buffer and resizing for " << numSamples << " samples");
    mpx_buffer.clear();
    mpx_buffer.resize(numSamples);

    initilized = true;
	TRACE("Exited Method");
    return 0;
}


/**
 * Here is what we need to do each pass
 * 1. Bring in the audio
 * 2. Add the RDS to the audio
 * 3. Add the pilot tone
 */
int Transmitter::doWork() {
	TRACE("Entered Method");

	TRACE("Receiving samples from fm_mpx_get_samples()");
	if( fm_mpx_get_samples(&mpx_buffer[0], &rds_status_struct, &fm_mpx_status_struct) < 0 ) {
		ERROR("Error occurred adding RDS data to sound file.");
		return -1;
	}

	TRACE("Scaling samples");
	// scale samples
	for(int i = 0; i < numSamples; i++) {
		mpx_buffer[i] /= 10.;
	}

	TRACE("Exited Method");
    return 0;
}


std::vector<float>& Transmitter::getData() {
	TRACE("Entered Method");
	TRACE("Returning vector float of size: " << mpx_buffer.size());
	TRACE("Exited Method");
	return mpx_buffer;
}


// From: http://stackoverflow.com/questions/1549930/c-equivalent-of-java-tostring
// Yes, that's right, I had to google that, sad face java developer. :-(
std::ostream& operator<<(std::ostream &strm, const Transmitter &tx) {
  return strm << std::endl
		  << "File Name: " << tx.filePath << std::endl
		  << "Center Frequency: " << tx.centerFrequency << std::endl
		  << "RDS String: " << tx.rdsText << std::endl;
}
