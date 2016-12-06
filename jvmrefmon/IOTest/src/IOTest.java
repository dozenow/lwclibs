import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class IOTest {

	static {
		System.loadLibrary("testiojni");
	}

	public static void main(String[] args) throws IOException {

		// confining after jvm init
		System.out.println("Confining after init");
		int rmfd = LWCNI.lwCConfine();

		// create 100 temporary files
		for (int i = 0; i < 10000; i++) {
			BufferedWriter out = new BufferedWriter(
					new FileWriter("/home/elnikety/workspace/snap/jvmrefmon/tmpfiles/" + i + ".iotest.txt"));
			out.write("Refmon, are you getting this?");
			out.close();
		}

		// read and double check
		for (int i = 0; i < 10000; i++) {
			BufferedReader in = new BufferedReader(new FileReader("tmpfiles/" + i + ".iotest.txt"));
			String l = in.readLine();
			assert (l.compareTo("Refmon, are you getting this?") == 0);
			in.close();
		}

		// remove
		for (int i = 0; i < 10000; i++) {
			new File("tmpfiles/" + i + ".iotest.txt").delete();
		}

		System.out.println("Test done!");
		LWCNI.lwCCleanup(rmfd);
	}
}
