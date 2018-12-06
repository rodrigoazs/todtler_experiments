package ranker.instance;

import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class InstanceComparator implements Comparator<Instance> {

	private static InstanceComparator INSTANCE;

	private InstanceComparator() {
		// NOP
	}

	public static InstanceComparator getInstance() {
		if (INSTANCE == null) {
			INSTANCE = new InstanceComparator();
		}
		return INSTANCE;
	}

	@Override
	public int compare(Instance instance1, Instance instance2) {
		if (instance1.getScore() < instance2.getScore()) {
			return 1;
		}
		else if (instance1.getScore() > instance2.getScore()) {
			return -1;
		}
		else {
			return 0;
		}
	}

}
