package util;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class implements a sorted list as an extension of an <code>ArrayList</code>.
 */
public class SortedList<T> extends ArrayList<T> {

	private static final long serialVersionUID = 1L;

	private final Comparator<T> comparator;

	public SortedList(Comparator<T> comparator) {
		this.comparator = comparator;
	}

	private Comparator<T> getComparator() {
		return this.comparator;
	}

	@Override
	public void add(int index, T element) {
		return;
	}

	@Override
	public boolean add(T object) {
		int index = 0;
		boolean found = false;
		while (!found && (index < this.size())) {
			found = this.getComparator().compare(object, this.get(index)) < 0;
			if (!found) {
				index++;
			}
		}
		super.add(index, object);
		return true;
	}

	@Override
	public boolean addAll(Collection<? extends T> objects) {
		boolean result = false;
		for (T object : objects) {
			result = this.add(object) || result;
		}
		return result;
	}

}
