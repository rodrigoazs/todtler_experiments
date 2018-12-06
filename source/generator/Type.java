package generator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents a type that can appear in a predicate.
 */
public class Type {

	/**
	 * The name of this type.
	 */
	private final String name;

	/**
	 * Whether or not this type can appear as a constant.
	 */
	private final boolean canAppearAsConstant;

	/**
	 * Constructs a type with the given name and qualifier whether it can appear as a constant or not.
	 * 
	 * @param name
	 *            The given name.
	 * @param canAppearAsConstant
	 *            The given qualifier whether it can appear as a constant or not.
	 */
	Type(final String name, final boolean canAppearAsConstant) {
		this.name = name;
		this.canAppearAsConstant = canAppearAsConstant;
	}

	public String getName() {
		return this.name;
	}

	public boolean canAppearAsConstant() {
		return this.canAppearAsConstant;
	}

}
