package util;

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class OutputLogger {

	private final File outputFile;

	public OutputLogger(final File outputFile) {
		this.outputFile = outputFile;
	}

	public boolean log(String message) {
		System.out.println(message);
		try {
			FileUtils.writeStringToFile(this.getOutputFile(), message + "\n", true);
		}
		catch (IOException e) {
			return false;
		}
		return true;
	}

	private File getOutputFile() {
		return this.outputFile;
	}

}
