import pkg from 'sequelize';
const { DataTypes } = pkg;

export default {
	projectID: {
		type: DataTypes.INTEGER,
		allowNull: false,
		primaryKey: true,
	},
	userID: {
		type: DataTypes.INTEGER,
		allowNull: false,
		primaryKey: true,
	},
	stars: {
		type: DataTypes.INTEGER,
		allowNull: false,
	},
};
