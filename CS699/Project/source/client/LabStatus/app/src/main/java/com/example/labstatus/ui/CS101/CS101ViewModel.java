package com.example.labstatus.ui.CS101;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

/** @brief Model for Cs101 grid data

Detailed description follows here.
 @author Area51
 @date Nov 2019
 */
public class CS101ViewModel extends ViewModel {

    private MutableLiveData<String> mText;
    public String text;

    public String color;

    public CS101ViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is Basement fragment");
    }
    /**
     * Constructor for Model
     * @param c color
     * @param t text
     */
    public CS101ViewModel(String t, String c) {

        text=t;
        color=c;
    }

    public LiveData<String> getText() {
        return mText;
    }
}