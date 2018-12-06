package util;

import java.io.File;
import java.util.List;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class Util {

	public static String convertFilesToString(List<File> files) {
		StringBuilder stringBuilder = new StringBuilder();
		String prefix = "";
		for (File file : files) {
			stringBuilder.append(prefix);
			stringBuilder.append(file.getAbsolutePath());
			prefix = ",";
		}
		return stringBuilder.toString();
	}

}
